
#include "../builtin.h"
#include "test.h"

TEST_SUITE_START(BuiltinTest);


// -----------------------------------------substr-----------------------------------------
dtStr *string = strNewFromCStr("0123456789");
dtStr *substring1;
substr(string, 1, 2, &substring1);
SHOULD_EQUAL("SubString1", strCmpCStr(substring1, "1"), 0);
strFree(substring1);

dtStr *substring2;
substr(string, 0, 5, &substring2);
SHOULD_EQUAL("SubString2 (beginIndex = 0)", strCmpCStr(substring2, "01234"), 0);
strFree(substring2);

dtStr *substring3;
substr(string, 5, 5, &substring3);
SHOULD_EQUAL("SubString3 (beginIndex = endIndex)", strCmpCStr(substring3, ""), 0);
strFree(substring3);

dtStr *substring4;
SHOULD_EQUAL("SubString4 (beginIndex > endIndex)", (substr(string, 6, 2, &substring4) == ERR_OTHER), 1);

dtStr *substring5;
SHOULD_EQUAL("SubString5 (endIndex > s->uiLength)", (substr(string, 1, 11, &substring5) == ERR_OTHER), 1);

dtStr *substring6;
SHOULD_EQUAL("SubString6 (beginIndex < 0)", (substr(string, -1, 5, &substring6) == ERR_OTHER), 1);

strFree(string);

// ----------------------------------------readInt----------------------------------------
tSymbolPtr symbol_1 = symbolNew();
symbol_1->Type = eINT;
ungetc('\n', stdin);
ungetc('1', stdin);
SHOULD_EQUAL("readInt(): 1", readData(symbol_1, &symbol_1->Data), ERR_OK);
SHOULD_EQUAL("readInt(): 1", symbol_1->Data.Integer, 1);
symbolFree(symbol_1);

tSymbolPtr symbol_2 = symbolNew();
symbol_2->Type = eINT;
char charArray2[] = "12345\n";
for (int i = strlen(charArray2); i >=0; i--)
{
	ungetc(charArray2[i], stdin);
}
SHOULD_EQUAL("readInt(): 12345", readData(symbol_2, &symbol_2->Data), ERR_OK);
SHOULD_EQUAL("readInt(): 12345", symbol_2->Data.Integer, 12345);
symbolFree(symbol_2);

tSymbolPtr symbol_3 = symbolNew();
symbol_3->Type = eINT;
ungetc('\n', stdin);
ungetc(' ', stdin);
SHOULD_NOT_EQUAL("readInt(): [space]", readData(symbol_3, &symbol_3->Data), ERR_OK);
symbolFree(symbol_3);

tSymbolPtr symbol_4 = symbolNew();
symbol_4->Type = eINT;
ungetc('\n', stdin);
ungetc('1', stdin);
ungetc(' ', stdin);
SHOULD_NOT_EQUAL("readInt(): [space]1", readData(symbol_4, &symbol_4->Data), ERR_OK);
symbolFree(symbol_4);

tSymbolPtr symbol_5 = symbolNew();
symbol_5->Type = eINT;
ungetc('\n', stdin);
ungetc('a', stdin);
SHOULD_NOT_EQUAL("readInt(): a", readData(symbol_5, &symbol_5->Data), ERR_OK);
symbolFree(symbol_5);

tSymbolPtr symbol_6 = symbolNew();
symbol_6->Type = eINT;
char charArray6[] = "michalj3najlepsi\n";
for (int i = strlen(charArray6); i >=0; i--)
{
	ungetc(charArray6[i], stdin);
}
SHOULD_NOT_EQUAL("readInt(): michalj3najlepsi", readData(symbol_6, &symbol_6->Data), ERR_OK);
symbolFree(symbol_6);

tSymbolPtr symbol_7 = symbolNew();
symbol_7->Type = eINT;
char charArray7[] = "42okicko\n";
for (int i = strlen(charArray7); i >=0; i--)
{
	ungetc(charArray7[i], stdin);
}
SHOULD_NOT_EQUAL("readInt(): 42okicko", readData(symbol_7, &symbol_7->Data), ERR_OK);
symbolFree(symbol_7);

tSymbolPtr symbol_8 = symbolNew();
symbol_8->Type = eINT;
char charArray8[] = "420 42\n";
for (int i = strlen(charArray8); i >=0; i--)
{
	ungetc(charArray8[i], stdin);
}
SHOULD_NOT_EQUAL("readInt(): 420 42", readData(symbol_8, &symbol_8->Data), ERR_OK);
symbolFree(symbol_8);

tSymbolPtr symbol_9 = symbolNew();
symbol_9->Type = eINT;
char charArray9[] = "421 ahoj\n";
for (int i = strlen(charArray9); i >=0; i--)
{
	ungetc(charArray9[i], stdin);
}
SHOULD_NOT_EQUAL("readInt(): 421 ahoj", readData(symbol_9, &symbol_9->Data), ERR_OK);
symbolFree(symbol_9);


//-----------------------------readDouble test-----------------------------

tSymbolPtr symbol_30 = symbolNew();
symbol_30->Type = eDOUBLE;
char charArray30[] = "3\n";
for (int i = strlen(charArray30)-1; i >= 0; i--) {
	ungetc(charArray30[i], stdin);
}
SHOULD_EQUAL("readDouble(): 3", readData(symbol_30, &symbol_30->Data), ERR_OK);
SHOULD_EQUAL("readDouble(): 3", symbol_30->Data.Double, 3);
symbolFree(symbol_30);


tSymbolPtr symbol_31 = symbolNew();
symbol_31->Type = eDOUBLE;
char charArray31[] = "3.1415\n";
for (int i = strlen(charArray31)-1; i >= 0; i--) {
	ungetc(charArray31[i], stdin);
}
SHOULD_EQUAL("readDouble(): 3.1415", readData(symbol_31, &symbol_31->Data), ERR_OK);
SHOULD_EQUAL("readDouble(): 3.1415", symbol_31->Data.Double, 3.1415);
symbolFree(symbol_31);


tSymbolPtr symbol_32 = symbolNew();
symbol_32->Type = eDOUBLE;
char charArray32[] = "3.1415e0\n";
for (int i = strlen(charArray32)-1; i >= 0; i--) {
	ungetc(charArray32[i], stdin);
}
SHOULD_EQUAL("readDouble(): 3.1415e0", readData(symbol_32, &symbol_32->Data), ERR_OK);
SHOULD_EQUAL("readDouble(): 3.1415e0", symbol_32->Data.Double, 3.1415e0);
symbolFree(symbol_32);


tSymbolPtr symbol_33 = symbolNew();
symbol_33->Type = eDOUBLE;
char charArray33[] = "31415956e-3\n";
for (int i = strlen(charArray33)-1; i >= 0; i--) {
	ungetc(charArray33[i], stdin);
}
SHOULD_EQUAL("readDouble(): 31415956e-3", readData(symbol_33, &symbol_33->Data), ERR_OK);
SHOULD_EQUAL("readDouble(): 31415956e-3", symbol_33->Data.Double, 31415956e-3);
symbolFree(symbol_33);


tSymbolPtr symbol_34 = symbolNew();
symbol_34->Type = eDOUBLE;
char charArray34[] = " 3.1415\n";
for (int i = strlen(charArray34)-1; i >= 0; i--) {
	ungetc(charArray34[i], stdin);
}
SHOULD_NOT_EQUAL("readDouble(): [space]3.1415", readData(symbol_34, &symbol_34->Data), ERR_OK);
symbolFree(symbol_34);


tSymbolPtr symbol_35 = symbolNew();
symbol_35->Type = eDOUBLE;
char charArray35[] = "-45\n";
for (int i = strlen(charArray35)-1; i >= 0; i--) {
	ungetc(charArray35[i], stdin);
}
SHOULD_EQUAL("readDouble(): -45", readData(symbol_35, &symbol_35->Data), ERR_OK);
SHOULD_EQUAL("readDouble(): -45", symbol_35->Data.Double, -45);
symbolFree(symbol_35);


tSymbolPtr symbol_36 = symbolNew();
symbol_36->Type = eDOUBLE;
char charArray36[] = "-3.14\n";
for (int i = strlen(charArray36)-1; i >= 0; i--) {
	ungetc(charArray36[i], stdin);
}
SHOULD_EQUAL("readDouble(): -3.14", readData(symbol_36, &symbol_36->Data), ERR_OK);
SHOULD_EQUAL("readDouble(): -3.14", symbol_36->Data.Double, -3.14);
symbolFree(symbol_36);


tSymbolPtr symbol_37 = symbolNew();
symbol_37->Type = eDOUBLE;
char charArray37[] = "-42e-10\n";
for (int i = strlen(charArray37)-1; i >= 0; i--) {
	ungetc(charArray37[i], stdin);
}
SHOULD_EQUAL("readDouble(): -42e-10", readData(symbol_37, &symbol_37->Data), ERR_OK);
SHOULD_EQUAL("readDouble(): -42e-10", symbol_37->Data.Double, -42e-10);
symbolFree(symbol_37);


tSymbolPtr symbol_38 = symbolNew();
symbol_38->Type = eDOUBLE;
char charArray38[] = "\n";
for (int i = strlen(charArray38)-1; i >= 0; i--) {
	ungetc(charArray38[i], stdin);
}
SHOULD_NOT_EQUAL("readDouble(): [empty]", readData(symbol_38, &symbol_38->Data), ERR_OK);
symbolFree(symbol_38);


tSymbolPtr symbol_39 = symbolNew();
symbol_39->Type = eDOUBLE;
char charArray39[] = " \n";
for (int i = strlen(charArray39)-1; i >= 0; i--) {
	ungetc(charArray39[i], stdin);
}
SHOULD_NOT_EQUAL("readDouble(): [space]", readData(symbol_39, &symbol_39->Data), ERR_OK);
symbolFree(symbol_39);


tSymbolPtr symbol_40 = symbolNew();
symbol_40->Type = eDOUBLE;
char charArray40[] = "123a5\n";
for (int i = strlen(charArray40)-1; i >= 0; i--) {
	ungetc(charArray40[i], stdin);
}
SHOULD_NOT_EQUAL("readDouble(): 123a5", readData(symbol_40, &symbol_40->Data), ERR_OK);
symbolFree(symbol_40);


tSymbolPtr symbol_41 = symbolNew();
symbol_41->Type = eDOUBLE;
char charArray41[] = "abcd\n";
for (int i = strlen(charArray41)-1; i >= 0; i--) {
	ungetc(charArray41[i], stdin);
}
SHOULD_NOT_EQUAL("readDouble(): abcd", readData(symbol_41, &symbol_41->Data), ERR_OK);
symbolFree(symbol_41);


tSymbolPtr symbol_42 = symbolNew();
symbol_42->Type = eDOUBLE;
char charArray42[] = "3. 42\n";
for (int i = strlen(charArray42)-1; i >= 0; i--) {
	ungetc(charArray42[i], stdin);
}
SHOULD_NOT_EQUAL("readDouble(): 3.[space]42", readData(symbol_42, &symbol_42->Data), ERR_OK);
symbolFree(symbol_42);


tSymbolPtr symbol_43 = symbolNew();
symbol_43->Type = eDOUBLE;
char charArray43[] = "10ee3\n";
for (int i = strlen(charArray43)-1; i >= 0; i--) {
	ungetc(charArray43[i], stdin);
}
SHOULD_NOT_EQUAL("readDouble(): 10ee3", readData(symbol_43, &symbol_43->Data), ERR_OK);
symbolFree(symbol_43);


tSymbolPtr symbol_44 = symbolNew();
symbol_44->Type = eDOUBLE;
char charArray44[] = "489-5\n";
for (int i = strlen(charArray44)-1; i >= 0; i--) {
	ungetc(charArray44[i], stdin);
}
SHOULD_NOT_EQUAL("readDouble(): 489-5", readData(symbol_44, &symbol_44->Data), ERR_OK);
symbolFree(symbol_44);





//--------------------------------------readString test--------------------------------------
tSymbolPtr symbol_str_1 = symbolNew();
symbol_str_1->Type = eSTRING;
char charArrayStr1[] = "Hello World\n";
for (int i = strlen(charArrayStr1); i >=0; i--)
{
	ungetc(charArrayStr1[i], stdin);
}
SHOULD_EQUAL("readString(): Hello World", readData(symbol_str_1, &symbol_str_1->Data) || strcmp(symbol_str_1->Data.String->str, "Hello World"), 0);
symbolFree(symbol_str_1);

tSymbolPtr symbol_str_2 = symbolNew();
symbol_str_2->Type = eSTRING;
char charArrayStr2[] = "\\\"Hello\\nWorld\\042\\tIFJ\\\\2016\n";
for (int i = strlen(charArrayStr2) ; i >= 0; i--)
{
	ungetc(charArrayStr2[i], stdin);
}
SHOULD_EQUAL("readString(): \\\"Hello\\nWorld\\042\\tIFJ\\\\2016", readData(symbol_str_2, &symbol_str_2->Data) || strcmp(symbol_str_2->Data.String->str, "\"Hello\nWorld\042\tIFJ\\2016"), 0);
symbolFree(symbol_str_2);

tSymbolPtr symbol_str_3 = symbolNew();
symbol_str_3->Type = eSTRING;
char charArrayStr3[] = "Hello\\aWorld\n";
for (int i = strlen(charArrayStr3); i >=0; i--)
{
	ungetc(charArrayStr3[i], stdin);
}
SHOULD_NOT_EQUAL("readString(): Hello\\aWorld", readData(symbol_str_3, &symbol_str_3->Data), ERR_OK);
symbolFree(symbol_str_3);

tSymbolPtr symbol_str_4 = symbolNew();
symbol_str_4->Type = eSTRING;
char charArrayStr4[] = "Hello\\22World\n";
for (int i = strlen(charArrayStr4); i >=0; i--)
{
	ungetc(charArrayStr4[i], stdin);
}
SHOULD_NOT_EQUAL("readString(): Hello\\22World", readData(symbol_str_4, &symbol_str_4->Data), ERR_OK);
symbolFree(symbol_str_4);

tSymbolPtr symbol_str_5 = symbolNew();
symbol_str_5->Type = eSTRING;
char charArrayStr5[] = "Hello\\2World\n";
for (int i = strlen(charArrayStr5); i >=0; i--)
{
	ungetc(charArrayStr5[i], stdin);
}
SHOULD_NOT_EQUAL("readString(): Hello\\2World", readData(symbol_str_5, &symbol_str_5->Data), ERR_OK);
symbolFree(symbol_str_5);

tSymbolPtr symbol_str_6 = symbolNew();
symbol_str_6->Type = eSTRING;
char charArrayStr6[] = "Hello\\9999World\n";
for (int i = strlen(charArrayStr6); i >=0; i--)
{
	ungetc(charArrayStr6[i], stdin);
}
SHOULD_NOT_EQUAL("readString(): Hello\\9999World", readData(symbol_str_6, &symbol_str_6->Data), ERR_OK);
symbolFree(symbol_str_6);


TEST_SUITE_END
