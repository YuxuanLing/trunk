/**********************************************************************************************************************
*	$Log: vsys#comm#inc#src#cfg.c_1,v $
*	Revision 1.10  2007-10-05 21:19:10-07  lsha
*	...No comments entered during checkin...
*
*	Revision 1.9  2007-08-27 18:50:55-07  lsha
*	Re-partition include files
*
*
*	DESCRIPTION:
*	This file provides APIs to read items from configuration text file (".cfg").
*
**********************************************************************************************************************/

#include	"cutil.h"



#ifndef	__CODE_LINK__
/**	__CODE_LINK__: all
 */

/**	SECTION - configuration reading APIs
 */
	void *_dbgCFG = NULL;							///	debug output file pointer
	char	arr2s8cfg[256][256];						/* Existing file names */
	SIGN32	s32cfg;										/* Number of existing file names */

	/******************************************************************************************************************
	*	Function: _cfgAPI_Format
	*	Prototype description: ../cutil.h
	******************************************************************************************************************/
	SIGN32	_cfgAPI_Format(
						char	*ps8txt,
						char	*ps8cfg,
						char	s8Keep,
						char	s8Remove,
						char	*ps8Begin,
						char	*ps8End,
						char	*arrps8fmt[],
						SIGN32	s32fmt
						)
	{
	SIGN32	i, s32word, bSection = (ps8Begin == NULL), fmt = - 1;
	char	arrs8line[KILO];

	FILE	*fpt = fopen(ps8txt, "rt"), *fpc;
	if(!fpt) {
		_fLOG(_dbgCFG, (CUTIL "input text file \"%s\" open failed!\n", ps8txt)); return ERR_FILE;
			}
	if(!(fpc = _filemk(ps8cfg, "wt"))) {
		_fLOG(_dbgCFG, (CUTIL "output configuration file \"%s\" open failed!\n", ps8cfg)); fclose(fpt); return ERR_FILE;
			}

	while((s32word = _cfgAPI_GetItem(arrs8line, fpt, 0, NULL, "", NULL, NULL)) >= 0) {
		if(s32word == 0) continue;
		if(garrps8[0][0] == s8Keep) { fprintf(fpc, "\n\n"); strcat(arrs8line, "\n"); }
		else {
			arrs8line[strlen(arrs8line) - 1] = NULL;
			if(ps8Begin && !_stricmp(garrps8[0], ps8Begin)) {
				fprintf(fpc, "\n\n"); bSection = 1; fmt = 0;
					}
			else {
				if(!bSection) continue;
				if(ps8End && !_stricmp(garrps8[0], ps8End)) {
					fprintf(fpc, "\n\n"); strcat(arrs8line, "\n\n"); bSection = 0; fmt = - 1;
						}
				else {
					if(garrps8[0][0] == s8Remove) {
						fmt = - 1; fprintf(fpc, "\n");
							}
					for(i = 0; i < s32fmt; i ++)
						if(garrps8[0][0] == arrps8fmt[i][0]) {
							fmt = i; fprintf(fpc, "%s", arrps8fmt[i] + 1); break;
								}
					if(fmt < 0) continue;
					if(i == s32fmt)
						if(fmt < s32fmt - 1) fprintf(fpc, "  ");
						else fprintf(fpc, "%s", arrps8fmt[fmt] + 1);
				}
			}
		}

		fprintf(fpc, "%s", arrs8line); fflush(fpc);
	}

	fprintf(fpc, "\n"); fclose(fpt); fclose(fpc); return SUCCESS;
	/**	ENDOFFUNCTION: _cfgAPI_Format **/
	}


	/******************************************************************************************************************
	*	Function: _cfgAPI_RecusiveParse
	*	Prototype description: ../cutil.h "_cfgAPI_PreProcess"
	******************************************************************************************************************/
	FILE*	_cfgAPI_RecusiveParse(
						char	*ps8cfg,
						char	*ps8option,
						SIGN32	bInsertLink
						)
	{
	char	arrs8[KILO], arrs8line[KILO];
	SIGN32	i, s32word, s32cur = s32cfg - 1;
	FILE	*fpin = fopen(ps8cfg, "rt"), *fpout, *fp;
	if(!fpin) {
		_fLOG(_dbgCFG, (CUTIL "input file \"%s\" open failed!\n", ps8cfg)); return NULL;
			}
	SplitPath(ps8cfg, arrs8line, garrs8); ModifyFileSuffix(garrs8, ".log");
	strcat(arrs8line, "tmp"); sprintf(arrs8, "%s/_%s", arrs8line, garrs8);
	if(!(fpout = _filemk(arrs8, "wt"))) {
		_fLOG(_dbgCFG, (CUTIL "output file \"%s\" open failed!\n", arrs8)); fclose(fpin); return NULL;
			}

	if(bInsertLink)
		fprintf(fpout, "$link\t\"%s\"\n", arr2s8cfg[s32cur]);
	while((s32word = _cfgAPI_GetItem(arrs8line, fpin, 0, NULL, "", NULL, NULL)) >= 0) {
		if(s32word == 0) continue;
		if(garrps8[0][0] != '#') { fputs(arrs8line, fpout); fflush(fpout); continue; }
		else if(!_stricmp(garrps8[0], "#option")) {
			for(i = 1; i < s32word; i ++)
				if(!_stricmp(garrps8[i], ps8option)) break;
			if((i < s32word) || (_cfgAPI_GetItem(NULL, fpin, 0, "#endoption", "", NULL, NULL) >= 0))
				continue;
			_fLOG(_dbgCFG, (CUTIL "missing #endoption!\n")); fclose(fpin); fclose(fpout); return NULL;
		}
		else if(!_stricmp(garrps8[0], "#require") && (s32word > 1)) {
			if(garrps8[1][0] == '#')
				strcpy(garrs8, garrps8[1] + 1);
			else {
				strcpy(garrs8, garrps8[1]);
				for(i = 0; i < s32cfg; i ++)
					if(!_stricmp(arr2s8cfg[i], garrs8)) break;
				if(i < s32cfg) { if(bInsertLink) fprintf(fpout, "$link\t\"%s\" #\n", garrs8); continue; }
					}
			strcpy(arr2s8cfg[s32cfg ++], garrs8);
			if(fp = _cfgAPI_RecusiveParse(garrs8, ps8option, bInsertLink)) {
				while(fgets(garrs8, KILO, fp)) fputs(garrs8, fpout);
				if(bInsertLink)
					fprintf(fpout, "$link\t\"%s\" return\n", arr2s8cfg[s32cur]);
				fflush(fpout); fclose(fp); continue;
					}
			_fLOG(_dbgCFG, (CUTIL "failed to include \"%s\"!\n", arr2s8cfg[s32cfg]));
			/* fclose(fpin); fclose(fpout); return NULL; */
		}
	}

	fclose(fpin); fclose(fpout); return fopen(arrs8, "rt");
	/**	ENDOFFUNCTION: _cfgAPI_RecusiveParse **/
	}



	/******************************************************************************************************************
	*	Function: _cfgAPI_PreProcess
	*	Prototype description: ../cutil.h
	******************************************************************************************************************/
	FILE*	_cfgAPI_PreProcess(
						char	*ps8cfg,
						char	*ps8option,
						SIGN32	bInsertLink
						)
	{
	strcpy(arr2s8cfg[0], ps8cfg); s32cfg = 1;
	
	return _cfgAPI_RecusiveParse(ps8cfg, ps8option, bInsertLink);
	/**	ENDOFFUNCTION: _cfgAPI_PreProcess **/
	}



	/******************************************************************************************************************
	*	Function: _cfgAPI_LineParsing
	*	Prototype description: ../cutil.h
	******************************************************************************************************************/
	SIGN32	_cfgAPI_LineParsing(
						char	*ps8str,
						FILE	*fpCFG,
						char	*ps8QuoteList,
						SIGN32	s32QuoteCount,
						char	*ps8SymbolList,
						SIGN32	s32SymbolCount,
						char	**pps8CommentList,
						SIGN32	s32CommentCount,
						char	**pps8word
						)
	{
	static	char	arrs8QuoteList[] = { '\"', '\'' };
	static	char	arrs8SymbolList[] = { '\n', '\r', ' ', '\t', '\"', '\'', ',', ';', '"', '"' };	/* 'â€?,'â€? */
	static	char	*arrps8CommentList[] = { "//", "/*" };
	SIGN32	i, j, n, s32quote = - 1, s32word = 0;
	char	*ps8comment;

	/* Prepare work set */
	if(!ps8str) ps8str = garrs8;
	if(fpCFG && !fgets(ps8str, KILO, fpCFG))
		return ERR_MISMATCH;
	if(!pps8word) pps8word = garrps8;
	if(garrs8 != ps8str) strcpy(garrs8, ps8str);
	if(garrps8 == pps8word) for(i = 0; i < 256; i ++) pps8word[i] = NULL;

	if(!ps8QuoteList && s32QuoteCount) {
		ps8QuoteList = arrs8QuoteList; s32QuoteCount = 2;				/* Use default */
			}
	if(!ps8SymbolList && s32SymbolCount) {
		ps8SymbolList = arrs8SymbolList; s32SymbolCount = 10;			/* Use default */
			}
	if(!pps8CommentList && s32CommentCount) {
		pps8CommentList = arrps8CommentList; s32CommentCount = 2;		/* Use default */
			}

	/* Delete all comments */
	for(i = 0; i < s32CommentCount; i ++)
		if(ps8comment = strstr(garrs8, pps8CommentList[i]))
			garrs8[ps8comment - garrs8] = 0;
	n = (SIGN32)strlen(garrs8);

	/* Parsing from head to tail */
	for(i = 0; i < n; i ++) {
		if(s32quote < 0) {
			for(j = 0; j < s32QuoteCount; j ++)
				if(garrs8[i] == ps8QuoteList[j]) { s32quote = j; break; }
		}
		else {
			if(garrs8[i] != ps8QuoteList[s32quote]) continue;			/* Ignoring sub-string inside a quote pair */
			s32quote = - 1;
		}

		for(j = 0; j < s32SymbolCount; j ++)							/* Detect word boundary */
			if(garrs8[i] == ps8SymbolList[j]) { garrs8[i] = 0; break; }
	}

	for(i = 0; i < n;) {
		if(garrs8[i]) {
			if(pps8word[s32word]) strcpy(pps8word[s32word], garrs8 + i);
			else pps8word[s32word] = garrs8 + i;
			s32word ++;
				}
		i += strlen(garrs8 + i) + 1;
	}

	return s32word;
	/**	ENDOFFUNCTION: _cfgAPI_LineParsing **/
	}



	/******************************************************************************************************************
	*	Function: _cfgAPI_GetItem
	*	Prototype description: ../cutil.h
	******************************************************************************************************************/
	SIGN32	_cfgAPI_GetItem(
						char	*ps8str,
						FILE	*fpCFG,
						SIGN32	bpfx,
						char	*ps8ItemName,
						char	*ps8ItemStop,
						char	**pps8word,
						char	*ps8SymbolList
						)
	{
	SIGN32	s32rtn = fpCFG ? 0 : ERR_FILE;
	if(!pps8word) pps8word = garrps8;
	if(!ps8ItemStop)
		fseek(fpCFG, 0, SEEK_SET);
	else if(!*ps8ItemStop) ps8ItemStop = NULL;

	while(s32rtn >= 0) {
		SIGN32	s32pos = (SIGN32)ftell(fpCFG), i = - 1; if(ps8SymbolList) i = strlen(ps8SymbolList);

		if((s32rtn = _cfgAPI_LineParsing(ps8str, fpCFG, NULL, - 1, ps8SymbolList, i, NULL, - 1, pps8word)) >= 0) {
			if(ps8ItemStop && (s32rtn > 0) && !_stricmp(ps8ItemStop, pps8word[0])) {
				fseek(fpCFG, s32pos, SEEK_SET); s32rtn = ERR_MISMATCH;
			}
			else if(!ps8ItemName) break;
			else if(s32rtn > 0) {
				if(!bpfx && !_stricmp(ps8ItemName, pps8word[0]))
					break;
				else if(bpfx && !memcmp(ps8ItemName, pps8word[0], strlen(ps8ItemName)))
					break;
			}
		}
	}

	_dftLOG(0, 31, (CUTIL "_cfgAPI_GetItem: "));
	if(s32rtn < 0)
		_dftLOG(1, 31, ("%s\n", eperr[- s32rtn]));
	else {
		if(ps8ItemName)
			_dftLOG(0, 31, ("%s found, ", ps8ItemName));
		_dftLOG(1, 31, ("%d word(s) parsed\n", s32rtn));
	}

	return s32rtn;
	/**	ENDOFFUNCTION: _cfgAPI_GetItem **/
	}



	/******************************************************************************************************************
	*	Function: _cfgAPI_GetItemString
	*	Prototype description: ../cutil.h
	******************************************************************************************************************/
	SIGN32	_cfgAPI_GetItemString(
						FILE	*fpCFG,
						char	*ps8ItemName,
						char	*ps8ItemStop,
						char	*ps8ItemString
						)
	{
	SIGN32	s32rtn = _cfgAPI_GetItem(NULL, fpCFG, 0, ps8ItemName, ps8ItemStop, NULL, "\n\r \t\"\',;=:");

	if(s32rtn < 0)
		return s32rtn;
	if(s32rtn < 2)
		return ERR_MISMATCH;

	if(ps8ItemString) {
		strcpy(ps8ItemString, garrps8[1]);
		_dftLOG(1, 30, (CUTIL "_cfgAPI_GetItemString: \"%s\"\n", ps8ItemString));
	}

	return SUCCESS;
	/**	ENDOFFUNCTION: _cfgAPI_GetItemString **/
	}



	/******************************************************************************************************************
	*	Function: _cfgAPI_GetItemValue
	*	Prototype description: ../cutil.h
	******************************************************************************************************************/
	SIGN32	_cfgAPI_GetItemValue(
						FILE	*fpCFG,
						char	*ps8ItemName,
						char	*ps8ItemStop,
						SIGN32	*ps32ItemValue
						)
	{
	SIGN32	s32rtn = _cfgAPI_GetItemString(fpCFG, ps8ItemName, ps8ItemStop, garrs8);

	if((s32rtn == SUCCESS) && ps32ItemValue)
		if(garrs8[1] == 'x') sscanf(garrs8 + 2, "%x", ps32ItemValue);
		else sscanf(garrs8, "%d", ps32ItemValue);
	return s32rtn;
	/**	ENDOFFUNCTION: _cfgAPI_GetItemValue **/
	}



	/******************************************************************************************************************
	*	Function: _cfgAPI_GetItemHexValue
	*	Prototype description: ../cutil.h
	******************************************************************************************************************/
	SIGN32	_cfgAPI_GetItemHexValue(
						FILE	*fpCFG,
						char	*ps8ItemName,
						char	*ps8ItemStop,
						UNSG32	*pu32ItemValue
						)
	{
	SIGN32	s32rtn = _cfgAPI_GetItemString(fpCFG, ps8ItemName, ps8ItemStop, garrs8);

	if((s32rtn == SUCCESS) && pu32ItemValue)
		sscanf(garrs8, "%x", pu32ItemValue);
	return s32rtn;
	/**	ENDOFFUNCTION: _cfgAPI_GetItemHexValue **/
	}



	/******************************************************************************************************************
	*	Function: _cfgAPI_GetItemRealValue
	*	Prototype description: ../cutil.h
	******************************************************************************************************************/
	SIGN32	_cfgAPI_GetItemRealValue(
						FILE	*fpCFG,
						char	*ps8ItemName,
						char	*ps8ItemStop,
						REAL64	*pf64ItemValue
						)
	{
	SIGN32	s32rtn = _cfgAPI_GetItemString(fpCFG, ps8ItemName, ps8ItemStop, garrs8);

	if((s32rtn == SUCCESS) && pf64ItemValue)
		sscanf(garrs8, "%f", pf64ItemValue);
	return s32rtn;
	/**	ENDOFFUNCTION: _cfgAPI_GetItemRealValue **/
	}



	/******************************************************************************************************************
	*	Function: _cfgAPI_FileMergeIn
	*	Prototype description: ../cutil.h
	******************************************************************************************************************/
	SIGN32	_cfgAPI_FileMergeIn(
						char	*ps8file,
						char	*ps8section,
						char	*ps8descMerge,
						char	*ps8commentPfx
						)
	{
	FILE	*fpw, *fpr;
	char	arrs8line[kilo], ps8merge[kilo];
	SIGN32	i;

	sprintf(ps8merge, "%s.txt", ps8section);
	if(!(fpr = fopen(ps8file, "rt"))) {
		xdbg(CUTIL "\n\n\"%s\" cannot be read!\n\n", ps8file);
		return ERR_FILE;
			}
	if(!(fpw = fopen(ps8merge, "wt"))) {
		xdbg(CUTIL "\n\n\"%s\" cannot be written!\n\n", ps8merge);
		fclose(fpr); return ERR_FILE;
			}
	do {
	i = _cfgAPI_GetItem(arrs8line, fpr, 0, NULL, ps8descMerge, NULL, NULL);
	fprintf(fpw, "%s", arrs8line);
	} while(i >= 0);

	fclose(fpr);
	if(!(fpr = fopen(ps8section, "rt"))) {
		xdbg(CUTIL "\n\n\"%s\" cannot be read!\n\n", ps8section);
		fclose(fpw); return ERR_FILE;
			}
	if(ps8commentPfx) {
		_api_curSec(garrs8);
		fprintf(fpw, "%s/* Last update: %s */\n", ps8commentPfx, garrs8);
			}
	while(_cfgAPI_GetItem(arrs8line, fpr, 0, NULL, "", NULL, NULL) >= 0) fprintf(fpw, "%s", arrs8line);

	fclose(fpr); fpr = fopen(ps8file, "rt");
	_cfgAPI_GetItem(arrs8line, fpr, 0, ps8descMerge, NULL, NULL, NULL);
	_cfgAPI_GetItem(arrs8line, fpr, 0, ps8descMerge, ps8descMerge, NULL, NULL);
	while(_cfgAPI_GetItem(arrs8line, fpr, 0, NULL, "", NULL, NULL) >= 0) fprintf(fpw, "%s", arrs8line);

	fclose(fpr); fclose(fpw);
	if(!(fpw = fopen(ps8file, "wt"))) {
		xdbg(CUTIL "\n\n\"%s\" cannot be written!\n\n", ps8file);
		return ERR_FILE;
			}
	fpr = fopen(ps8merge, "rt");
	while(_cfgAPI_GetItem(arrs8line, fpr, 0, NULL, "", NULL, NULL) >= 0) fprintf(fpw, "%s", arrs8line);

	fclose(fpr); fclose(fpw); _unlink(ps8merge); return SUCCESS;
	/**	ENDOFFUNCTION: _cfgAPI_FileMergeIn **/
	}

/**	ENDOFSECTION
 */

/**	__CODE_LINK__: all
 */
#endif



/**	ENDOFFILE: cfg.c **************************************************************************************************
 */

