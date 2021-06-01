#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

enum class RegistrationStatus{
    Unknown,
    Registered,
    Error
};

class GSMModuleConfiguration{
public:
    virtual std::vector<std::string> getConfiguration() =0;
    virtual ~GSMModuleConfiguration() =default;
};

class GSMModule{
public:
    virtual bool configure(void *_config) =0;
    virtual bool isReady() =0;
    virtual uint32_t getSignal() =0;
    virtual RegistrationStatus getRegistrationStatus() =0;
    virtual void reset() =0;
    virtual bool sendSms(const std::string_view& _mobile_number, const std::string_view& text) =0;
    virtual std::string readSms(const uint32_t& _index) =0;
    virtual std::string getSmsSender(const uint32_t& _index) =0;
    virtual bool deleteAllSms() =0;
	virtual ~GSMModule() =default;
};
