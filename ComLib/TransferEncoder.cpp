#include "TransferEncoder.h"
#include "json/json.h"

using namespace std;
using namespace Json;

/*
{
    "cmd":"event",
        "content":{
            "type":"proc_add",
                "data":{
                    "added":[
                    {
                        "unique":12345,
                        "pid":1234,
                        "procPath":"d:\\abcdef.exe",
                        "procDesc":"desc",
                        "cmd":"abcdef",
                        "startTime":"2018-11-11 11:11:11:123",
                        "x64":1,
                        "session":1,
                        "user":"DESKTOP-DCTRL5K\\Administrator",
                        "sid":"S-1-5-21-2669793992-3689076831-3814312677-500"
                    },
                    ...
                    ],
                    "killed":[
                        1111,2222,3333
                    ]
            }
    }
}
*/
utf8_mstring __stdcall EncodeProcMon(const ProcInfoSet &procSet) {
    Value root, added(arrayValue), killed(arrayValue);
    for (list<ProcMonInfo>::const_iterator it = procSet.mAddSet.begin() ; it != procSet.mAddSet.end() ; it++)
    {
        Value node;
        node["unique"] = (UINT)it->procUnique;
        node["pid"] = (UINT)it->procPid;
        node["procPath"] = it->procPath;
        node["procDesc"] = it->procDesc;
        node["cmd"] = it->procCmd;
        node["startTime"] = it->startTime;
        node["x64"] = int(it->x64);
        node["session"] = (UINT)it->sessionId;
        node["user"] = it->procUser;
        node["sid"] = it->procUserSid;
        added.append(node);
    }
    root["added"] = added;

    for (list<DWORD>::const_iterator ij = procSet.mKillSet.begin() ; ij != procSet.mKillSet.end() ; ij++)
    {
        killed.append((UINT)*ij);
    }
    root["killed"] = killed;
    return FastWriter().write(root);
}

ProcInfoSet __stdcall DecodeProcMon(const utf8_mstring &json) {
    ProcInfoSet result;
    Value root, added, killed;
    Reader().parse(json, root);
    added = root["added"], killed = root["killed"];

    for (size_t i = 0 ; i != added.size() ; i++)
    {
        Value node = added[i];
        ProcMonInfo info;
        info.procUnique = node["unique"].asUInt();
        info.procPid = node["pid"].asUInt();
        info.procPath = node["procPath"].asString();
        info.procDesc = node["procDesc"].asString();
        info.procCmd = node["cmd"].asString();
        info.startTime = node["startTime"].asString();
        info.x64 = node["x64"].asInt();
        info.sessionId = node["session"].asInt();
        info.procUser = node["user"].asString();
        info.procUserSid = node["sid"].asString();
        result.mAddSet.push_back(info);
    }

    for (size_t j = 0 ; j < killed.size() ; j++)
    {
        result.mKillSet.push_back(killed[j].asUInt());
    }
    return result;
}

std::utf8_mstring _declspec(dllexport) __stdcall EncodeProcCreate(const ProcCreateInfo &info) {
    Value content;
    content["pid"] = (UINT)info.mPid;
    content["image"] = info.mImage;
    content["baseAddr"] = info.mBaseAddr;
    content["entryAddr"] = info.mEntryAddr;
    return FastWriter().write(content);
}

ProcCreateInfo _declspec(dllexport) __stdcall DecodeProcCreate(const std::utf8_mstring &json) {
    ProcCreateInfo info;
    Value content;
    Reader().parse(json, content);

    info.mPid = content["pid"].asUInt();
    info.mImage = content["image"].asString();
    info.mBaseAddr = content["baseAddr"].asString();
    info.mEntryAddr = content["entryAddr"].asString();
    return info;
}

/*
{
    "cmd":"event",
    "content":{
        "type":"moduleload",
        "data":{
            "name":"kernel32.dll",
            "baseAddr":"0x4344353",
            "endAddr":"0x43443ff"
        }
    }
}
*/
std::mstring _declspec(dllexport) __stdcall EncodeDllLoadInfo(const DllLoadInfo &info) {
    Value data;
    data["name"] = info.mDllName;
    data["baseAddr"] = info.mBaseAddr;
    data["endAddr"] = info.mEndAddr;

    return FastWriter().write(data);
}

DllLoadInfo _declspec(dllexport) __stdcall DecodeDllLoadInfo(const std::mstring &json) {
    DllLoadInfo info;
    Value content;
    Reader().parse(json, content);

    info.mDllName = content["name"].asString();
    info.mBaseAddr = content["baseAddr"].asString();
    info.mEndAddr = content["endAddr"].asString();
    return info;
}

mstring _declspec(dllexport) __stdcall EncodeCmdRegister(const RegisterContent &context) {
    return "{}";
}

RegisterContent _declspec(dllexport) __stdcall DecodeCmdRegister(const std::mstring &json) {
    return RegisterContent();
}

/*
{
    "cmd": "reply",
    "content": {
        "status": 0,
        "reason": "abcdef",
        "result":{
            "cmdCode":0,
            "cmdShow":"abcd1234",
            "cmdResult": [{
                "addr": "0x0xabcd12ff",
                "function":"kernel32!CreateFileW",
                "param0": "0xabcd1234",
                "param1": "0xabcd1234",
                "param2": "0xabcd1233",
                "param3": "0xabcd1233"
            }]
        } 
    }
}
*/
mstring _declspec(dllexport) __stdcall EncodeCmdCallStack(const CallStackData &callStack) {
    Value result(arrayValue);

    for (list<CallStackSingle>::const_iterator it = callStack.mCallStack.begin() ; it != callStack.mCallStack.end() ; it++)
    {
        Value single;
        single["addr"] = it->mAddr;
        single["function"] = it->mFunction;
        single["param0"] = it->mParam0;
        single["param1"] = it->mParam1;
        single["param2"] = it->mParam2;
        single["param3"] = it->mParam3;
        result.append(single);
    }
    return FastWriter().write(result);
}

CallStackData __stdcall DecodeCmdCallStack(const mstring &json) {
    CallStackData data;
    Value result;

    Reader().parse(json, result);
    for (int i = 0 ; i < (int)result.size() ; i++)
    {
        Value singleJson = result[i];
        CallStackSingle single;
        single.mAddr = singleJson["addr"].asString();
        single.mFunction = singleJson["function"].asString();
        single.mParam0 = singleJson["param0"].asString();
        single.mParam1 = singleJson["param1"].asString();
        single.mParam2 = singleJson["param2"].asString();
        single.mParam3 = singleJson["param3"].asString();
        data.mCallStack.push_back(single);
    }
    return data;
}