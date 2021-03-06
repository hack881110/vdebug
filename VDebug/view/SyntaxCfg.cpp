#include <Windows.h>
#include <map>
#include <string>
#include <fstream>
#include <ComLib/ComLib.h>
#include "SyntaxCfg.h"
#include <ComLib/ComLib.h>
#include "../SyntaxHlpr/SyntaxTextView.h"

using namespace std;

static map<mstring, int> *gs_pSyntaxMap = NULL;
static map<int, SyntaxColourDesc *> *gs_pSyntaxCfg = NULL;
static bool gs_bInit = false;

static DWORD gs_defTextColour = 0;
static DWORD gs_defBackColour = 0;
static DWORD gs_CaretLineColour = 0;
static DWORD gs_SelTextColour = 0;
static DWORD gs_SelBackColour = 0;
static DWORD gs_SelAlpha = 0;

SyntaxColourDesc *GetSyntaxCfg(int type)
{
    map<int, SyntaxColourDesc *>::const_iterator it = gs_pSyntaxCfg->find(type);
    if (gs_pSyntaxCfg->end() != it)
    {
        return it->second;
    }
    return NULL;
}

static SyntaxColourDesc *_GetDescFromJson(const mstring &key, const Value &json)
{
    SyntaxColourDesc *desc = new SyntaxColourDesc();
    desc->m_dwTextColour = GetColourFromStr(json["textColour"].asString().c_str());
    desc->m_dwBackColour = GetColourFromStr(json["backColour"].asString().c_str());
    desc->m_bBold = json["bold"].asInt();
    desc->m_bItalic = json["italic"].asInt();

    if (desc->m_dwTextColour == NULL_COLOUR)
    {
        desc->m_dwTextColour = gs_defTextColour;
    }

    if (desc->m_dwBackColour == NULL_COLOUR)
    {
        desc->m_dwBackColour = gs_defBackColour;
    }
    desc->m_strDesc = key;
    return desc;
}

static void _InitSyntaxCfg() {
    gs_pSyntaxMap = new map<mstring, int>();
    gs_pSyntaxCfg = new map<int, SyntaxColourDesc *>();

    gs_pSyntaxMap->insert(make_pair("default", STYLE_DBG_DEFAULT));
    gs_pSyntaxMap->insert(make_pair("addr", STYLE_DBG_ADDR));
    gs_pSyntaxMap->insert(make_pair("register", STYLE_DBG_REGISTER));
    gs_pSyntaxMap->insert(make_pair("error", STYLE_DBG_ERROR));
    gs_pSyntaxMap->insert(make_pair("message", STYLE_DBG_MESSAGE));
    gs_pSyntaxMap->insert(make_pair("hex", STYLE_DBG_HEX));
    gs_pSyntaxMap->insert(make_pair("data", STYLE_DBG_DATA));
    gs_pSyntaxMap->insert(make_pair("byte", STYLE_DBG_BYTE));
    gs_pSyntaxMap->insert(make_pair("inst", STYLE_DBG_INST));
    gs_pSyntaxMap->insert(make_pair("call", STYLE_DBG_CALL));
    gs_pSyntaxMap->insert(make_pair("jmp", STYLE_DBG_JMP));
    gs_pSyntaxMap->insert(make_pair("proc", STYLE_DBG_PROC));
    gs_pSyntaxMap->insert(make_pair("module", STYLE_DBG_MODULE));
    gs_pSyntaxMap->insert(make_pair("param", STYLE_DBG_PARAM));
    gs_pSyntaxMap->insert(make_pair("keyword", STYLE_DBG_KEYWORD));
    gs_pSyntaxMap->insert(make_pair("number", STYLE_DBG_NUMBER));
    gs_pSyntaxMap->insert(make_pair("hight", STYLE_DBG_HIGHT));
}

BOOL UpdateSyntaxView(SyntaxTextView *pSyntaxView) {
    pSyntaxView->SetDefStyle(gs_defTextColour, gs_defBackColour);
    pSyntaxView->ShowCaretLine(true, gs_CaretLineColour, 100);
    //pSyntaxView->SendMsg(SCI_SETSELFORE, 1, gs_SelTextColour);
    pSyntaxView->SendMsg(SCI_SETSELBACK, 1, gs_SelBackColour);
    pSyntaxView->SendMsg(SCI_SETSELALPHA, gs_SelAlpha, 0);

    map<int, SyntaxColourDesc *>::const_iterator it;
    for (it = gs_pSyntaxCfg->begin() ; it != gs_pSyntaxCfg->end() ; it++)
    {
        SyntaxColourDesc *desc = it->second;
        pSyntaxView->SetStyle(it->first, desc->m_dwTextColour, desc->m_dwBackColour);
    }
    return TRUE;
}

BOOL LoadSyntaxCfg(const string &path)
{
    if (!gs_bInit)
    {
        gs_bInit = true;

        _InitSyntaxCfg();
    }

    PFILE_MAPPING_STRUCT pMapping = NULL;
    BOOL stat = FALSE;

    do
    {
        pMapping = MappingFileA(path.c_str());
        if (pMapping == NULL || pMapping->hFile == INVALID_HANDLE_VALUE)
        {
            break;
        }

        Value root;
        Reader().parse((const char *)pMapping->lpView, root);

        if (root.type() != objectValue)
        {
            break;
        }

        //Global Config
        Value golbal = root["globalCfg"];
        Value cfg = root["syntaxCfg"];

        /*
        {
            "lineNumber": 1,
            "defTextColour": "0,0,255",
            "defBackColour": "40,40,40",
            "viewBackColour": "40,40,40",
            "curLineColour": "60,56,54",
            "selTextColour": "255,0,0",
            "selBackColour": "0,255,0",
            "selAlpha":100
        }
        */
        gs_defTextColour = GetColourFromStr(GetStrFormJson(golbal, "defTextColour").c_str());
        gs_defBackColour = GetColourFromStr(GetStrFormJson(golbal, "defBackColour").c_str());
        gs_CaretLineColour = GetColourFromStr(GetStrFormJson(golbal, "curLineColour").c_str());
        gs_SelTextColour = GetColourFromStr(GetStrFormJson(golbal, "selTextColour").c_str());
        gs_SelBackColour = GetColourFromStr(GetStrFormJson(golbal, "selBackColour").c_str());
        gs_SelAlpha = GetIntFromJson(golbal, "selAlpha");

        for (Json::Value::iterator it = cfg.begin() ; it != cfg.end() ; it++) {
            SyntaxColourDesc *desc = _GetDescFromJson(it.key().asString(), *it);

            map<mstring, int>::const_iterator ij = gs_pSyntaxMap->find(desc->m_strDesc);
            if (ij != gs_pSyntaxMap->end())
            {
                gs_pSyntaxCfg->insert(make_pair(ij->second, desc));
            }
        }
    } while (FALSE);

    if (pMapping)
    {
        CloseFileMapping(pMapping);
    }
    return stat;
}