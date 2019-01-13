//==============================================================================
/**
@file       MyStreamDeckPlugin.h

@brief      CPU plugin

@copyright  (c) 2018, Corsair Memory, Inc.
			This source code is licensed under the MIT-style license found in the LICENSE file.

**/
//==============================================================================

#include "Common/ESDBasePlugin.h"
#include <mutex>

class CpuUsageHelper;
class VoiceMeeterHelper;
class CallBackTimer;

class Settings {
public:
	Settings(int stripNumber): mStripNumber(stripNumber) {		
	};
	virtual ~Settings() {};

	int mStripNumber;
};

class MyStreamDeckPlugin : public ESDBasePlugin
{
public:
	
	MyStreamDeckPlugin();
	virtual ~MyStreamDeckPlugin();
	
	void KeyDownForAction(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID) override;
	void KeyUpForAction(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID) override;
	
	void WillAppearForAction(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID) override;
	void WillDisappearForAction(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID) override;
	
	void DeviceDidConnect(const std::string& inDeviceID, const json &inDeviceInfo) override;
	void DeviceDidDisconnect(const std::string& inDeviceID) override;

	void SendToPlugin(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID) override;

private:
	
	void UpdateTimer(bool force);
	
	std::mutex mVisibleContextsMutex;
	std::map<std::string, Settings> mVisibleContexts;
	
	CpuUsageHelper *mCpuUsageHelper = nullptr;
	VoiceMeeterHelper *mVoiceMeeterHelper = nullptr;
	CallBackTimer *mTimer;
};
