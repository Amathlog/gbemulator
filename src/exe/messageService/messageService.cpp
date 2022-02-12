#include <exe/messageService/messageService.h>

using GBEmulatorExe::IMessageService;
using GBEmulatorExe::DispatchMessageService;
using GBEmulatorExe::Message;

bool DispatchMessageService::Push(const Message& message)
{
    bool pushOK = true;
    for (auto* messageService : m_messageServices)
        pushOK &= messageService->Push(message);

    return pushOK;
}

bool DispatchMessageService::Pull(Message& message)
{
    bool pullOK = true;
    for (auto* messageService : m_messageServices)
        pullOK &= messageService->Pull(message);

    return pullOK;
}

void DispatchMessageService::Connect(IMessageService* messageService)
{
    m_messageServices.insert(messageService);
}

void DispatchMessageService::Disconnect(IMessageService* messageService)
{
    m_messageServices.erase(messageService);
}