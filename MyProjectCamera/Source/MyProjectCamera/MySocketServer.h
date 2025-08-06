#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "MySocketServer.generated.h"

UCLASS()
class MYPROJECTCAMERA_API AMySocketServer : public AActor
{
    GENERATED_BODY()

public:
    AMySocketServer();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaTime) override;
    virtual void BeginDestroy() override;

    // ✅ 누락된 함수 선언 추가
    bool StartTCPListener(const FString& SocketName, const FString& IP, const int32 Port);
    void ProcessMessage(const FString& Message);
    void ShutdownSocket();

    // ✅ 누락된 변수 선언 추가
    FSocket* ListenerSocket;
    FSocket* ConnectionSocket;
    TSharedPtr<FInternetAddr> RemoteAddress;
    ISocketSubsystem* SocketSubsystem = nullptr;

    UPROPERTY(EditAnywhere, Category = "Socket")
    int32 Port = 7777;

};
