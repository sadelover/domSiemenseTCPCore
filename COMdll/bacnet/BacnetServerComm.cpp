// BacnetComm.cpp 
#include "stdafx.h"
#include "BacnetServerComm.h"

#include "Tools/Util/UtilString.h"
#include <process.h>  //about thread

#include "../LAN_WANComm/NetworkComm.h"

#pragma warning(disable:4267)
//////////////////////////////////////////////////////////////////////////

#include "../bacnet/include/tsm.h"
#include "../bacnet/include/address.h"
#include "../bacnet/include/device.h"
#include "../bacnet/include/datalink.h"
#include "../bacnet/include/handlers.h"
#include "../bacnet/include/client.h"
#include "../bacnet/include/dlenv.h"
#include "../bacnet/include/whois.h"
#include "../bacnet/include/iam.h"
#include "../bacnet/include/txbuf.h"
#include "../bacnet/include/abort.h"
#include "../bacnet/include/bacerror.h"
#include "../bacnet/include/reject.h"
#include "../bacnet/include/bacdef.h"


object_functions_t Object_TableList_Server[25];

/* buffer used for receive */
static uint8_t Rx_Buf[MAX_MPDU] = { 0 };

CBacnetServerCtrl::CBacnetServerCtrl()
	: m_receive_thread(NULL)
	, m_bExitReceive(false)
{

}

CBacnetServerCtrl::~CBacnetServerCtrl()
{
	if(m_receive_thread != NULL)
	{
		CloseHandle(m_receive_thread);
		m_receive_thread = NULL;
	}

}

void CBacnetServerCtrl::InitObjectPoint0()
{

}

bool CBacnetServerCtrl::GetDeviceObjectInstance(uint32_t object_instance)
{
	return true;
}

int CBacnetServerCtrl::ReadDeviceObjects(BACNET_READ_PROPERTY_DATA * rp_data)
{
	//rp_data->object_instance = 2;
	rp_data->object_property = PROP_OBJECT_LIST;
	rp_data->object_type = DEVICE_OBJ_FUNCTIONS;

	//encode_bacnet_object_id()
	return 0;//BACNET_TYPE(rp_data->object_instance);
}

char * CBacnetServerCtrl::GetObjectName0(uint32_t object_instance)
{
	string strName = "temp88";
	char* c;
	const int len = strName.length();
	c = new char[len+1];

	strcpy(c,strName.c_str());

	return c;
}

void CBacnetServerCtrl::Init_Service_Handlers()
{
	Device_Init();
	/* we need to handle who-is to support dynamic device binding */
	apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_IS, handler_who_is);
	apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_HAS, handler_who_has);
	/* handle i-am to support binding to other devices */
	apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_I_AM, handler_i_am_bind);
	/* set the handler for all the services we don't implement */
	/* It is required to send the proper reject message... */
	apdu_set_unrecognized_service_handler_handler
		(handler_unrecognized_service);
	/* Set the handlers for any confirmed services that we support. */
	/* We must implement read property - it's required! */
	apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROPERTY,
		handler_read_property);
	apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROP_MULTIPLE,
		handler_read_property_multiple);
	apdu_set_confirmed_handler(SERVICE_CONFIRMED_WRITE_PROPERTY,
		handler_write_property);
//	apdu_set_confirmed_handler(SERVICE_CONFIRMED_WRITE_PROP_MULTIPLE,
//		handler_write_property_multiple);
	apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_RANGE,
		handler_read_range);
	apdu_set_confirmed_handler(SERVICE_CONFIRMED_REINITIALIZE_DEVICE,
		handler_reinitialize_device);
	apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_UTC_TIME_SYNCHRONIZATION,
		handler_timesync_utc);
	apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_TIME_SYNCHRONIZATION,
		handler_timesync);
	apdu_set_confirmed_handler(SERVICE_CONFIRMED_SUBSCRIBE_COV,
		handler_cov_subscribe);
	apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_COV_NOTIFICATION,
		handler_ucov_notification);
	/* handle communication so we can shutup when asked */
	apdu_set_confirmed_handler(SERVICE_CONFIRMED_DEVICE_COMMUNICATION_CONTROL,
		handler_device_communication_control);
	/* handle the data coming back from private requests */
	//apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_PRIVATE_TRANSFER,
	//	handler_unconfirmed_private_transfer);
}




void CBacnetServerCtrl::SetPointList( const vector<DataPointEntry>& entrylist )
{
	m_vecpointlist = entrylist;
}


bool CBacnetServerCtrl::StartBacnetServer()
{
    Device_Set_Object_Instance_Number(437);
    /* load any static address bindings to show up
       in our device bindings list */
    address_init(0);
    Init_Service_Handlers();
    dlenv_init();
    atexit(datalink_cleanup);
    /* broadcast an I-Am on startup */
    Send_I_Am(&Handler_Transmit_Buffer[0]);

	
	CreateThreadReceive();

	return true;
}

void	CBacnetServerCtrl::Exit()
{

}

bool CBacnetServerCtrl::CreateThreadReceive()
{

	/* configure the timeout values */

	if(m_receive_thread ==NULL)
	{
		m_receive_thread = (HANDLE)_beginthreadex(NULL,
			0,
			ThreadReceive, 
			(LPVOID)this,
			NORMAL_PRIORITY_CLASS,// CREATE_SUSPENDED,
			NULL);
	}	
	return true;
}

bool CBacnetServerCtrl::ExitBacnetServer()
{
	m_bExitReceive = true;

	if(m_receive_thread != NULL)
	{
		CloseHandle(m_receive_thread);
		m_receive_thread = NULL;
	}

	return true;
}

UINT WINAPI CBacnetServerCtrl::ThreadReceive( LPVOID pParam )
{
	CBacnetServerCtrl *pThis = static_cast<CBacnetServerCtrl*>(pParam);
	if (NULL == pThis)
		return 1;

	//init params
	BACNET_ADDRESS src = {0};
	uint16_t pdu_len = 0;
	 unsigned timeout = 100;     /* milliseconds */


	 uint32_t address_binding_tmr = 0;
	 time_t current_seconds = 0;
	 uint32_t elapsed_seconds = 0;
	 uint32_t elapsed_milliseconds = 0;
	 uint32_t recipient_scan_tmr = 0;

	 time_t last_seconds = time(NULL);


	while(!pThis->GetReceiveThreadExit())
	{
		pdu_len = datalink_receive(&src, &Rx_Buf[0], MAX_MPDU, timeout);
		
		/* process */
		if (pdu_len) {
			npdu_handler(&src, &Rx_Buf[0], pdu_len);
		}

		/* blink LEDs, Turn on or off outputs, etc */

	}
	return 0;
}

bool CBacnetServerCtrl::GetReceiveThreadExit()
{
	return m_bExitReceive;
}

void CBacnetServerCtrl::ResponseIAmHandler( uint8_t * service_request,uint16_t service_len,BACNET_ADDRESS * src )
{
	int len = 0;
	uint32_t device_id = 0;
	unsigned max_apdu = 0;
	int segmentation = 0;
	uint16_t vendor_id = 0;

	(void) src;
	(void) service_len;
	len =
		iam_decode_service_request(service_request, &device_id, &max_apdu,
		&segmentation, &vendor_id);
	fprintf(stderr, "Received I-Am Request");
	if (len != -1) {
		fprintf(stderr, " from %u!\n", device_id);
		address_add(device_id, max_apdu, src);
	} else
		fprintf(stderr, "!\n");

	return;
}

void CBacnetServerCtrl::ResponseWhoIsHandler( uint8_t * service_request,uint16_t service_len,BACNET_ADDRESS * src )
{
	int len = 0;
	int32_t low_limit = 0;
	int32_t high_limit = 0;

	(void) src;
	len =
		whois_decode_service_request(service_request, service_len, &low_limit,
		&high_limit);
	if (len == 0)
		Send_I_Am(&Handler_Transmit_Buffer[0]);
	else if (len != -1) {
		/* is my device id within the limits? */
		if (((Device_Object_Instance_Number() >= (uint32_t) low_limit) &&
			(Device_Object_Instance_Number() <= (uint32_t) high_limit))
			||
			/* BACnet wildcard is the max instance number - everyone responds */
			((BACNET_MAX_INSTANCE >= (uint32_t) low_limit) &&
			(BACNET_MAX_INSTANCE <= (uint32_t) high_limit)))
			Send_I_Am(&Handler_Transmit_Buffer[0]);
	}

	return;
}


void CBacnetServerCtrl::ResponseReadProperty(
	uint8_t * service_request,
	uint16_t service_len,
	BACNET_ADDRESS * src,
	BACNET_CONFIRMED_SERVICE_DATA * service_data)
{
	BACNET_READ_PROPERTY_DATA rpdata;
	int len = 0;
	int pdu_len = 0;
	int apdu_len = -1;
	int npdu_len = -1;
	BACNET_NPDU_DATA npdu_data;
	bool error = true;  /* assume that there is an error */
	int bytes_sent = 0;
	BACNET_ADDRESS my_address;

	/* configure default error code as an abort since it is common */
	rpdata.error_code = ERROR_CODE_ABORT_SEGMENTATION_NOT_SUPPORTED;
	/* encode the NPDU portion of the packet */
	datalink_get_my_address(&my_address);
	npdu_encode_npdu_data(&npdu_data, false, MESSAGE_PRIORITY_NORMAL);
	npdu_len =
		npdu_encode_pdu(&Handler_Transmit_Buffer[0], src, &my_address,
		&npdu_data);
	if (service_data->segmented_message) {
		/* we don't support segmentation - send an abort */
		len = BACNET_STATUS_ABORT;

		fprintf(stderr, "RP: Segmented message.  Sending Abort!\n");

		goto RP_FAILURE;
	}
	len = rp_decode_service_request(service_request, service_len, &rpdata);

	if (len <= 0) {
		fprintf(stderr, "RP: Unable to decode Request!\n");
	}

	if (len < 0) {
		/* bad decoding - skip to error/reject/abort handling */
		error = true;

		fprintf(stderr, "RP: Bad Encoding.\n");

		goto RP_FAILURE;
	}
	apdu_len =
		rp_ack_encode_apdu_init(&Handler_Transmit_Buffer[npdu_len],
		service_data->invoke_id, &rpdata);
	/* configure our storage */
	rpdata.application_data = &Handler_Transmit_Buffer[npdu_len + apdu_len];
	rpdata.application_data_len =
		sizeof(Handler_Transmit_Buffer) - (npdu_len + apdu_len);
	len = Device_Read_Property(&rpdata);
	if (len >= 0) {
		apdu_len += len;
		len =
			rp_ack_encode_apdu_object_property_end(&Handler_Transmit_Buffer
			[npdu_len + apdu_len]);
		apdu_len += len;
		if (apdu_len > service_data->max_resp) {
			/* too big for the sender - send an abort */
			len = BACNET_STATUS_ABORT;

			fprintf(stderr, "RP: Message too large.\n");

		} else {

			fprintf(stderr, "RP: Sending Ack!\n");

			error = false;
		}
	}

RP_FAILURE:
	if (error) {
		if (len == BACNET_STATUS_ABORT) {
			apdu_len =
				abort_encode_apdu(&Handler_Transmit_Buffer[npdu_len],
				service_data->invoke_id,
				abort_convert_error_code(rpdata.error_code), true);

			fprintf(stderr, "RP: Sending Abort!\n");

		} else if (len == BACNET_STATUS_ERROR) {
			apdu_len =
				bacerror_encode_apdu(&Handler_Transmit_Buffer[npdu_len],
				service_data->invoke_id, SERVICE_CONFIRMED_READ_PROPERTY,
				rpdata.error_class, rpdata.error_code);

			fprintf(stderr, "RP: Sending Error!\n");

		} else if (len == BACNET_STATUS_REJECT) {
			apdu_len =
				reject_encode_apdu(&Handler_Transmit_Buffer[npdu_len],
				service_data->invoke_id,
				reject_convert_error_code(rpdata.error_code));

			fprintf(stderr, "RP: Sending Reject!\n");

		}
	}

	pdu_len = npdu_len + apdu_len;
	bytes_sent =
		datalink_send_pdu(src, &npdu_data, &Handler_Transmit_Buffer[0],
		pdu_len);

	if (bytes_sent <= 0) {
		fprintf(stderr, "ERROR: Failed to send PDU (%s)!\n", strerror(errno));
	}

	bytes_sent = bytes_sent;


	return;
}

void CBacnetServerCtrl::SetLogSession( Beopdatalink::CLogDBAccess* logsesssion )
{
	m_logsession = logsesssion;
}
