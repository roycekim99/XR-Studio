#include "MySocketServer.h"
#include "EngineUtils.h"
#include "HAL/RunnableThread.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Common/TcpSocketBuilder.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

AMySocketServer::AMySocketServer()
{
    PrimaryActorTick.bCanEverTick = true;
}

bool AMySocketServer::StartTCPListener(const FString& SocketName, const FString& IP, const int32 InPort)
{
    SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
    RemoteAddress = SocketSubsystem->CreateInternetAddr();

    bool bIsValid;
    RemoteAddress->SetIp(*IP, bIsValid);
    RemoteAddress->SetPort(InPort);

    // ⬇️ 여기서 변환 처리
    uint32 RawIP = 0;
    RemoteAddress->GetIp(RawIP);

 
    FIPv4Address Addr(RawIP);

    ListenerSocket = FTcpSocketBuilder(*SocketName)
        .AsReusable()
        .BoundToAddress(Addr)
        .BoundToPort(InPort)
        .Listening(8);

    if (!ListenerSocket)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ 소켓 생성 실패"));
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("✅ TCP 서버 시작 (%s:%d)"), *IP, InPort);
    return true;
}


void AMySocketServer::BeginPlay()
{
    Super::BeginPlay();
    StartTCPListener(TEXT("MySocketListener"), TEXT("127.0.0.1"), 7777);
}

void AMySocketServer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!ListenerSocket) return;

    bool bPending;
    ListenerSocket->HasPendingConnection(bPending);

    if (bPending)
    {
        ConnectionSocket = ListenerSocket->Accept(TEXT("MyIncomingConnection"));

        if (ConnectionSocket)
        {
            TArray<uint8> ReceivedDataArray;
            uint32 Size;

            while (ConnectionSocket->HasPendingData(Size))
            {
                ReceivedDataArray.SetNumUninitialized(FMath::Min(Size, 65507u));
                int32 Read = 0;
                ConnectionSocket->Recv(ReceivedDataArray.GetData(), ReceivedDataArray.Num(), Read);
            }

            FString Received = FString(ANSI_TO_TCHAR(reinterpret_cast<const char*>(ReceivedDataArray.GetData())));
            UE_LOG(LogTemp, Warning, TEXT("📩 수신: %s"), *Received);

            ProcessMessage(Received);
            ConnectionSocket->Close();
            ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ConnectionSocket);
        }
    }
}

void AMySocketServer::ProcessMessage(const FString& Message)
{
    TArray<FString> Tokens;
    Message.ParseIntoArrayWS(Tokens);

    if (Tokens.Num() >= 5 && Tokens[0] == "MOVE")
    {
        FString ActorName = Tokens[1];
        float X = FCString::Atof(*Tokens[2]);
        float Y = FCString::Atof(*Tokens[3]);
        float Z = FCString::Atof(*Tokens[4]);

        for (TActorIterator<AActor> It(GetWorld()); It; ++It)
        {
            if (It->GetName().Contains(ActorName))
            {
                It->SetActorLocation(FVector(X, Y, Z));
                UE_LOG(LogTemp, Warning, TEXT("✅ %s 이동 완료: (%.1f, %.1f, %.1f)"), *ActorName, X, Y, Z);
                return;
            }
        }

        UE_LOG(LogTemp, Error, TEXT("❌ '%s' 이름의 액터를 찾을 수 없음"), *ActorName);
    }
}

void AMySocketServer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ShutdownSocket();
    Super::EndPlay(EndPlayReason);
}

void AMySocketServer::ShutdownSocket()
{
    if (ListenerSocket)
    {
        ListenerSocket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ListenerSocket);
        ListenerSocket = nullptr;
    }
}

void AMySocketServer::BeginDestroy()
{
    if (ListenerSocket) 
    {
        ShutdownSocket();
    }

    Super::BeginDestroy();
}