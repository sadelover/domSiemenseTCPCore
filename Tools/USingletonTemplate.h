#ifndef SINGLETON_H
#define SINGLETON_H
#pragma once

#include "CustomTools/CustomTools.h"

#include "stdlib.h"

/**
This template class is used to implement a singleton object.
*/

template<typename T>
class USingletonTemplate
{
public:
	// Get the object.
	static T& GetInstance() 
	{
		if (_instance == NULL)
		{

			Project::Tools::Scoped_Lock<Project::Tools::Mutex> lock(mutex);
			if (_instance == NULL)
			{
				_instance = new T();
				atexit(Destroy);
			}
		}

		return *_instance;
	}

protected:
	USingletonTemplate() {};
	// destructor
	~USingletonTemplate(){};
private:
	// constructor

	USingletonTemplate(const USingletonTemplate&);
	USingletonTemplate& operator=(const USingletonTemplate&);

	static void Destroy()
	{
		if (_instance != NULL)
		{
			delete _instance;
			_instance = NULL;
		}
	}
	// data members
	static Project::Tools::Mutex mutex;
	static T* volatile _instance;
};

template<typename T>
Project::Tools::Mutex USingletonTemplate<T>::mutex;

template<typename T>
T* volatile USingletonTemplate<T>::_instance= NULL;

#endif


