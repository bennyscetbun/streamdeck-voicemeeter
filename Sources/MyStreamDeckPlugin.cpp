//==============================================================================
/**
@file       MyStreamDeckPlugin.cpp

@brief      CPU plugin

@copyright  (c) 2018, Corsair Memory, Inc.
			This source code is licensed under the MIT-style license found in the LICENSE file.

**/
//==============================================================================

#include "MyStreamDeckPlugin.h"
#include <atomic>

#ifdef __APPLE__
	#include "macOS/CpuUsageHelper.h"
#else
	#include "Windows/CpuUsageHelper.h"
	#include "Windows/VoiceMeeterHelper.h"
#endif

#include "Common/EPLJSONUtils.h"
#include "Common/ESDConnectionManager.h"

class CallBackTimer
{
public:
    CallBackTimer() :_execute(false) { }

    ~CallBackTimer()
    {
        if( _execute.load(std::memory_order_acquire) )
        {
            stop();
        };
    }

    void stop()
    {
        _execute.store(false, std::memory_order_release);
        if(_thd.joinable())
            _thd.join();
    }

    void start(int interval, std::function<void(void)> func)
    {
        if(_execute.load(std::memory_order_acquire))
        {
            stop();
        };
        _execute.store(true, std::memory_order_release);
        _thd = std::thread([this, interval, func]()
        {
            while (_execute.load(std::memory_order_acquire))
            {
                func();
                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
            }
        });
    }

    bool is_running() const noexcept
    {
        return (_execute.load(std::memory_order_acquire) && _thd.joinable());
    }

private:
    std::atomic<bool> _execute;
    std::thread _thd;
};

MyStreamDeckPlugin::MyStreamDeckPlugin()
{
	mCpuUsageHelper = new CpuUsageHelper();
	mVoiceMeeterHelper = new VoiceMeeterHelper();
	mTimer = new CallBackTimer();
	mTimer->start(200, [this]()
	{
		this->UpdateTimer(false);
	});
}

MyStreamDeckPlugin::~MyStreamDeckPlugin()
{
	if(mTimer != nullptr)
	{
		mTimer->stop();
		
		delete mTimer;
		mTimer = nullptr;
	}
	
	if(mCpuUsageHelper != nullptr)
	{
		delete mCpuUsageHelper;
		mCpuUsageHelper = nullptr;
	}
	if (mVoiceMeeterHelper != nullptr)
	{
		delete mVoiceMeeterHelper;
		mVoiceMeeterHelper = nullptr;
	}
}

void MyStreamDeckPlugin::UpdateTimer(bool force)
{
	//
	// Warning: UpdateTimer() is running in the timer thread
	//
	if(mConnectionManager != nullptr)
	{
		if (force == false && mVoiceMeeterHelper->Poll() == false)
			return;
		mVisibleContextsMutex.lock();
		for (std::pair<std::string, Settings> context : mVisibleContexts)
		{
			
			if (mVoiceMeeterHelper->GetMute(context.second.mStripNumber) < 0.5f)
				mConnectionManager->SetImage("data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEgAAABICAIAAADajyQQAAAACXBIWXMAAAsTAAALEwEAmpwYAAAAB3RJTUUH4wEMACUU2ZtRHgAAAB1pVFh0Q29tbWVudAAAAAAAQ3JlYXRlZCB3aXRoIEdJTVBkLmUHAAAAWElEQVRo3u3PAREAAAQEsKd/Z3JwW4NVJi91IiYmJiYmJiYmJiYmJiYmJiYmJiYmJiYmJiYmJiYmJiYmJiYmJiYmJiYmJiYmJiYmJiYmJiYmJiYmJiZ20wI01AGPvGUuRAAAAABJRU5ErkJggg==", context.first, kESDSDKTarget_HardwareAndSoftware);
			else 
				mConnectionManager->SetImage("data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEgAAABICAIAAADajyQQAAAACXBIWXMAAAsTAAALEwEAmpwYAAAAB3RJTUUH4wEMACcsw6+LAgAAAB1pVFh0Q29tbWVudAAAAAAAQ3JlYXRlZCB3aXRoIEdJTVBkLmUHAAAAV0lEQVRo3u3PAREAAAQEsKd/Z3JwW4PV5KeOmJiYmJiYmJiYmJiYmJiYmJiYmJiYmJiYmJiYmJiYmJiYmJiYmJiYmJiYmJiYmJiYmJiYmJiYmJiYmNhRCzXTAY8v69aFAAAAAElFTkSuQmCC", context.first, kESDSDKTarget_HardwareAndSoftware);
		}
		mVisibleContextsMutex.unlock();
	}
}

void MyStreamDeckPlugin::KeyDownForAction(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID)
{
	// Nothing to do
}

void MyStreamDeckPlugin::KeyUpForAction(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID)
{
	mVisibleContextsMutex.lock();
	try {
		mVoiceMeeterHelper->ToggleMute(mVisibleContexts.at(inContext).mStripNumber);
	}
	catch(...){}
	mVisibleContextsMutex.unlock();
}

void MyStreamDeckPlugin::WillAppearForAction(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID)
{
	json settingsPayload;
	int stripNumber = 0;
	if (EPLJSONUtils::GetObjectByName(inPayload, "settings", settingsPayload)) {
		if (mConnectionManager != nullptr)
			mConnectionManager->SendToPropertyInspector(inAction, inContext, settingsPayload);
		stripNumber = EPLJSONUtils::GetIntByName(settingsPayload, "strip_number", 0);
	}
	// Remember the context
	mVisibleContextsMutex.lock();
	mVisibleContexts.insert( std::pair<std::string, Settings>(inContext, Settings(stripNumber)) );
	mVisibleContextsMutex.unlock();
	UpdateTimer(true);
}

void MyStreamDeckPlugin::WillDisappearForAction(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID)
{
	// Remove the context
	mVisibleContextsMutex.lock();
	mVisibleContexts.erase(inContext);
	mVisibleContextsMutex.unlock();
}

void MyStreamDeckPlugin::DeviceDidConnect(const std::string& inDeviceID, const json &inDeviceInfo)
{
	// Nothing to do
}

void MyStreamDeckPlugin::DeviceDidDisconnect(const std::string& inDeviceID)
{
	// Nothing to do
}

void MyStreamDeckPlugin::SendToPlugin(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID)
{
	if (EPLJSONUtils::GetBoolByName(inPayload, "DATAREQUEST", false)) {
		if (mConnectionManager == nullptr)
			return ;
		mVisibleContextsMutex.lock();
		try {
			json jsonObject;
			jsonObject["strip_number"] = mVisibleContexts.at(inContext).mStripNumber;
			mConnectionManager->SendToPropertyInspector(inAction, inContext, jsonObject);
		}
		catch (...) {
		}
		mVisibleContextsMutex.unlock();
		return;
	}


	int stripNumber = EPLJSONUtils::GetIntByName(inPayload, "strip_number", -1);
	if (stripNumber < 0) {
		return ;
	}
	if (mConnectionManager != nullptr)
		mConnectionManager->SetSettings(inPayload, inContext);
	Settings *setting;
	mVisibleContextsMutex.lock();
	try {
		setting = &mVisibleContexts.at(inContext);
	}
	catch (...) {
		mVisibleContextsMutex.unlock();
		return;
	}
	mVisibleContextsMutex.unlock();
	setting->mStripNumber = stripNumber;
	UpdateTimer(true);
}
