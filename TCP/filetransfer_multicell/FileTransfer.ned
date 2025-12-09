package simu5g.apps.filetransfer_multicell;

import inet.applications.contract.IApp;

simple FileSender like IApp
{
    parameters:
        @class(::FileSender);
        int localPort = default(-1);
        int destPort;
        string destAddress;
        int nFrames = default(10000); //0.005s interval, total time 40s -> 8000cnt < 10000cnt
        int payloadSize @unit(B) = default(500B);
        double samplingTime @unit(s) = default(0.005s);
        double startTime @unit(s) = default(1s); // warmup time 1sec


    gates:
        input socketIn;
        output socketOut;
}

simple FileReceiver like IApp
{
    parameters:
        @class(::FileReceiver); // ★ 전역 네임스페이스의 C++ 클래스 사용
        int localPort;		// set in omnetpp.ini

    gates:
        input socketIn;
        output socketOut;
}