#include "Extension.h"
#include <fstream>
#include <locale>
#include <codecvt>
#include <algorithm>
#include <functional>
#include <regex>

//Globals
std::wstring tlContents_utf16;
int tableLastEntryData = 0;
int tableLastEntryCombat = 0;
int tableLastEntryNames = 0;

struct tlDataRow {
	std::wstring tlJapanese;
	std::wstring tlEnglish;
};
struct tlRegexRow {
	int matchNum = 0;
	std::wregex tlRegex;
	std::wstring tlJapanese;
	std::wstring tlEnglish;
};

tlDataRow tlDataTable[50000];
tlRegexRow tlDataCombat[50000];
tlDataRow tlDataNames[50000];

bool sortComp(tlDataRow a, tlDataRow b) {
	return a.tlJapanese.size() > b.tlJapanese.size();
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	std::string tlContents_utf8;
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

	bool commentSkipMode = FALSE;
	wchar_t curC;
	int columnIndex = 0;
	int rowIndex = 0;


	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:

		//TL Data
		//Load the patch in utf-8 format and converts it to utf-16
		if (tlContents_utf16.empty()) {
			std::ifstream tlFile("./DDS Translation Patch//Translation.txt");
			if (tlFile.is_open()) {
				tlFile.seekg(0, std::ios::end);
				tlContents_utf8.resize(tlFile.tellg());
				tlFile.seekg(0, std::ios::beg);
				tlFile.read(&tlContents_utf8[0], tlContents_utf8.size());
				tlFile.close();
			}
			tlContents_utf16 = converter.from_bytes(tlContents_utf8);
		}

		//load data into table
		for (int i = 0; i < tlContents_utf16.size(); i++) {
			curC = tlContents_utf16[i];
			if (curC == L'#') { commentSkipMode = TRUE; continue; }
			if (curC == L'\n') { columnIndex = 0; rowIndex++; commentSkipMode = FALSE; continue; }
			if (!commentSkipMode) {
				if (curC == L'\t') { columnIndex=1; continue; }
				if (columnIndex == 0) {
					tlDataTable[rowIndex].tlJapanese += curC;
				}
				else {
					tlDataTable[rowIndex].tlEnglish += curC;
				}
				tableLastEntryData = rowIndex;
			}
		}

		//Combat strings
		//Load Combat strings
		tlContents_utf8.erase();
		tlContents_utf16.erase();
		commentSkipMode = FALSE;
		columnIndex = 0;
		rowIndex = 0;

		if (tlContents_utf16.empty()) {
			std::ifstream tlFileCombat("./DDS Translation Patch//Combat.txt");
			if (tlFileCombat.is_open()) {
				tlFileCombat.seekg(0, std::ios::end);
				tlContents_utf8.resize(tlFileCombat.tellg());
				tlFileCombat.seekg(0, std::ios::beg);
				tlFileCombat.read(&tlContents_utf8[0], tlContents_utf8.size());
				tlFileCombat.close();
			}
			tlContents_utf16 = converter.from_bytes(tlContents_utf8);
		}

		//load data into table
		for (int i = 0; i < tlContents_utf16.size(); i++) {
			curC = tlContents_utf16[i];
			if (curC == L'#') { commentSkipMode = TRUE; continue; }
			if (curC == L'\n') { columnIndex = 0; rowIndex++; commentSkipMode = FALSE; continue; }
			if (!commentSkipMode) {
				if (curC == L'\t') { columnIndex=1; continue; }
				if (columnIndex == 0) {
					tlDataCombat[rowIndex].tlJapanese += curC;
				}
				else {
					tlDataCombat[rowIndex].tlEnglish += curC;
				}
				tableLastEntryCombat = rowIndex;
			}
		}

		//Initialize regex
		for (int i = 0; i < tableLastEntryCombat; i++) {
			tlDataCombat[i].tlRegex = std::wregex(tlDataCombat[i].tlJapanese);
		}


		//Name Data
		//Load the patch in utf-8 format and converts it to utf-16
		//Also load them as part of tlDataTable
		tlContents_utf8.erase();
		tlContents_utf16.erase();
		commentSkipMode = FALSE;
		columnIndex = 0;
		rowIndex = 0;

		if (tlContents_utf16.empty()) {
			std::ifstream tlFileName("./DDS Translation Patch//Names.txt");
			if (tlFileName.is_open()) {
				tlFileName.seekg(0, std::ios::end);
				tlContents_utf8.resize(tlFileName.tellg());
				tlFileName.seekg(0, std::ios::beg);
				tlFileName.read(&tlContents_utf8[0], tlContents_utf8.size());
				tlFileName.close();
			}
			tlContents_utf16 = converter.from_bytes(tlContents_utf8);
		}

		//load data into table
		for (int i = 0; i < tlContents_utf16.size(); i++) {
			curC = tlContents_utf16[i];
			if (curC == L'#') { commentSkipMode = TRUE; continue; }
			if (curC == L'\n') { columnIndex = 0; rowIndex++; commentSkipMode = FALSE; continue; }
			if (!commentSkipMode) {
				if (curC == L'\t') { columnIndex=1; continue; }
				if (columnIndex == 0) {
					tlDataNames[rowIndex].tlJapanese += curC;
					tlDataTable[rowIndex + tableLastEntryData + 1].tlJapanese += curC;
				}
				else {
					tlDataNames[rowIndex].tlEnglish += curC;
					tlDataTable[rowIndex + tableLastEntryData + 1].tlEnglish += curC;
				}
				tableLastEntryNames = rowIndex;
			}
		}

		//Sort tlData + Names
		tableLastEntryData = tableLastEntryData + tableLastEntryNames + 1;
		std::sort(tlDataTable, tlDataTable + tableLastEntryData, sortComp);

		//Sort Names
		std::sort(tlDataNames, tlDataNames + tableLastEntryNames, sortComp);


		MessageBoxW(NULL, L"Extension Loaded", L"Example", MB_OK);
		break;
	case DLL_PROCESS_DETACH:
		MessageBoxW(NULL, L"Extension Removed", L"Example", MB_OK);
		break;
	}
	return TRUE;
}

//#define COPY_CLIPBOARD
//#define EXTRA_NEWLINES

/*
	Param sentence: sentence received by Textractor (UTF-16). Can be modified, Textractor will receive this modification only if true is returned.
	Param sentenceInfo: contains miscellaneous info about the sentence (see README).
	Return value: whether the sentence was modified.
	Textractor will display the sentence after all extensions have had a chance to process and/or modify it.
	The sentence will be destroyed if it is empty or if you call Skip().
	This function may be run concurrently with itself: please make sure it's thread safe.
	It will not be run concurrently with DllMain.
*/

bool MatchString(std::wstring match, wchar_t* target_char, int compareOffset, int compareLength) {
	if (compareOffset == match.size()) { return TRUE; }
	
	std::wstring target = target_char;
	int result = match.compare(compareOffset, compareLength,target);

	if (result == 0) { return TRUE; }
	return FALSE;
}

bool sortTranslationString(tlDataRow sortA, tlDataRow sortB, std::wstring originalMessage) {
	return originalMessage.find(sortA.tlJapanese) < originalMessage.find(sortB.tlJapanese);
}

bool ProcessRegexSentence(std::wstring& sentence) {

	int matchIndex;
	std::wsmatch regexM;
	std::wstring finalString;

	for (int i = 0; i < tableLastEntryCombat; i++) {
		if (tlDataCombat[i].tlJapanese.size() == 0) { continue; }
		if (!std::regex_search(sentence, tlDataCombat[i].tlRegex)) { continue; }


		if (std::regex_search(sentence, regexM, tlDataCombat[i].tlRegex)) {
			for (int j = 1; j < regexM.size(); j++) {
				for (int n = 0; n < tableLastEntryNames; n++) {
					if (regexM[j] == tlDataNames[n].tlJapanese) { regexM[j] == tlDataNames[n].tlEnglish; break; }
				}
			}
			finalString += regexM.format(tlDataCombat[i].tlEnglish);
			finalString += L"\n";
		}
	}

	if (finalString.size() == 0) { return false; }
	else { sentence = sentence + L"\n" + finalString; return true; }

}

bool ProcessSentence(std::wstring& sentence, SentenceInfo sentenceInfo)
{
	// Return if no sentence to process
	if (sentence.size() == 0) {
		return false;
	}

	//String finder variables
	std::wstring comString;
	wchar_t argString[1000] = {0};
	int tlIndex = 0;
	tlDataRow tlStrings[1000];


	//Output
	std::wstring outString;

	comString = sentence;	
	int matchIndex;

	//Check if string should be processed using Combat strings + regex
	if (ProcessRegexSentence(sentence)) { return true; }

	//If not, process normally
	for (int i = 0; i < tableLastEntryData; i++) {
		if (tlDataTable[i].tlJapanese.size() == 0) { continue; }

		matchIndex = comString.find(tlDataTable[i].tlJapanese);
		if (matchIndex == std::wstring::npos) { continue; }

		tlStrings[tlIndex].tlEnglish = tlDataTable[i].tlEnglish;
		tlStrings[tlIndex].tlJapanese = tlDataTable[i].tlJapanese;
		comString.replace(matchIndex, tlDataTable[i].tlJapanese.size(), L"\0");
		i--;
		tlIndex++;
	}

	outString = L"";

	if (tlIndex == 0) {
		return false;
	}

	//Sort list so that everything gets printed in the right order
	std::sort(tlStrings, tlStrings + tlIndex, std::bind(sortTranslationString, std::placeholders::_1, std::placeholders::_2, sentence));

	for (int i = 0; i < tlIndex; i++) {
		if (tlStrings[i].tlJapanese.size() == 0) { continue; }
		//Japanese
		outString += tlStrings[i].tlJapanese;
		outString += L" - ";
		//English
		outString += tlStrings[i].tlEnglish;
		outString += L"\n";
	}

	sentence = comString + L"\n" + outString + L"\n";

	return true;

#ifdef COPY_CLIPBOARD
	// This example extension automatically copies sentences from the hook currently selected by the user into the clipboard.
	if (sentenceInfo["current select"])
	{
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (sentence.size() + 2) * sizeof(wchar_t));
		memcpy(GlobalLock(hMem), sentence.c_str(), (sentence.size() + 2) * sizeof(wchar_t));
		GlobalUnlock(hMem);
		OpenClipboard(0);
		EmptyClipboard();
		SetClipboardData(CF_UNICODETEXT, hMem);
		CloseClipboard();
	}
	return false;
#endif // COPY_CLIPBOARD

#ifdef EXTRA_NEWLINES
	// This example extension adds extra newlines to all sentences.
	sentence += L"\r\n";
	return true;
#endif // EXTRA_NEWLINES
}

__declspec(dllexport) void test_ProcessSentence(std::wstring& sentence) {
	SentenceInfo sentenceInfo = { 0 };
	ProcessSentence(sentence, sentenceInfo);
}