#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

enum class RegistrationStatus{
    Unknown,
    Registered,
    Error
};

enum class ProcessResult{
    ERROR,
    OK
};

enum class ModuleState{
    Unknown,
    Idle,
    NewSms,
    SendingSms,
    IncomingCall,
    CallInProgress,
    Invalid
};

class GSMModuleConfiguration{
public:
    virtual std::vector<std::string> getConfiguration() =0;
    virtual ~GSMModuleConfiguration() =default;
};

class GSMModule{
public:
    virtual ProcessResult configure(void *_config) =0;
    virtual bool isReady() =0;
    virtual ModuleState getState() =0;
    virtual uint32_t getSignal() =0;
    virtual RegistrationStatus getNetworkRegistrationStatus() =0;
    virtual ProcessResult reset() =0;
    virtual ProcessResult sendMessage(const std::string_view& _mobile_number, const std::string_view& text) =0;
    virtual std::string readMessage(const uint32_t& _index) =0;
    virtual std::string getMessageOrigin(const uint32_t& _index) =0;
    virtual ProcessResult deleteAllMessages() =0;
	virtual ~GSMModule() =default;
};
