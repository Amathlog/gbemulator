#pragma once
#include <exe/messageService/message.h>
#include <set>

namespace GBEmulatorExe 
{
    class IMessageService
    {
    public:
        virtual ~IMessageService() = default;

        virtual bool Push(const Message& message) = 0;
        virtual bool Pull(Message& message) = 0;
    };

    class DispatchMessageService : public IMessageService
    {
    public:
        DispatchMessageService() = default;

        bool Push(const Message& message) override;
        bool Pull(Message& message) override;

        void Connect(IMessageService* messageService);
        void Disconnect(IMessageService* messageService);

    private:
        std::set<IMessageService*> m_messageServices;
    };

    class DispatchMessageServiceSingleton : public DispatchMessageService
    {
    public:
        static DispatchMessageServiceSingleton& GetInstance()
        {
            static DispatchMessageServiceSingleton s_instance;
            return s_instance;
        }

    private:
        DispatchMessageServiceSingleton() = default;
    };
}