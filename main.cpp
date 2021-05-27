#include <iostream>
#include <string>
#include <queue>
#include <fstream>

using namespace std;
int Register[10] = {0, 9, 5, 7, 1, 2, 3, 4, 5, 6};
int dataMemory[5] = {5, 9, 4, 8, 7};
queue<string> allInstructions;

bool IF = true;
bool ID = false;
bool EX = false;
bool MEM = false;
bool WB = false;
bool hazardTriggered = false;

int PC = 0;
int brancNum = 0;
string instruction = "00000000000000000000000000000000";

int readData1 = 0;
int readData2 = 0;
string sign_ext = "0000000000000000";
int Rs = 0;
int Rt = 0;
int Rd = 0;
string IDcontrolSignal = "000000000";

int EXALUout = 0;
int writeData = 0;
int EXRtRd = 0;
int EXRt = 0;
string EXcontrolSignal = "00000";

int readData = 0;
int MEMALUout = 0;
int MEMRtRd = 0;
string MEMcontrolSignal = "00";

int WBRtRd = 0;
int backData = 0;
string WBcontrolSignal = "00";

void instructionFetch(string input);
void instructionDecode();
void execution();
void memory();
void writeBack();

void instructionMemory(string opCode);
void general();
void branchHazardDetection();
void hazardDetectionUnit();
void forwardingUnit();
void ALU();
string ALUcontrol();
int AND(string data1, string data2);
int OR(string data1, string data2);

int binaryToDecimal(string str);
string decimalToBinary(int number);
int characterToInteger(char ch);
char IntegerToChacacter(int num);
void print(int clockCycle, int i);
void reset();


int main()
{
    for(int i=0;i<4;i++){
        ifstream Fin;
        switch(i){
        case 0:
            Fin.open("General.txt");
            break;
        case 1:
            Fin.open("Datahazard.txt");
            break;
        case 2:
            Fin.open("Lwhazard.txt");
            break;
        case 3:
            Fin.open("Branchhazard.txt");
            break;
        }
        if(!Fin){
            cout << "Cannot open the file " << endl;
        }
        reset();
        int clockCycle = 0;
        string input = "";
        while(getline(Fin, input)){
            if(input == ""){
                break;
            }
            allInstructions.push(input);
        }
        allInstructions.push("00000000000000000000000000000000");
        while(IF || ID || EX || MEM){
            clockCycle++;
            if(WB == true){
                writeBack();
            }
            if(MEM == true){
                memory();
            }
            if(EX == true && hazardTriggered == false){
                execution();
            }
            if(ID == true && hazardTriggered == false){
                instructionDecode();
            }
            PC = PC + 4;
            if(IF == true && hazardTriggered == false){
                instructionFetch(allInstructions.front());
                allInstructions.pop();
            }
            else if(IF == false && hazardTriggered == true){
                PC = PC - 4;
                EXcontrolSignal = "00000";
                hazardTriggered = false;
                IF = true;
                ID = true;
                EX = true;
                MEM = true;
                WB = true;
                general();
            }
            if(hazardTriggered == false){
                general();
            }
            print(clockCycle, i);
            if(brancNum != 0){
                PC = PC + brancNum - 4;
                brancNum = 0;
            }
        }
        Fin.close();
    }

    return 0;
}

void instructionFetch(string input){
    instruction = input;
    hazardDetectionUnit();
    branchHazardDetection();
}

void instructionDecode(){
    instructionMemory(instruction.substr(0, 6));
    if(instruction == "00000000000000000000000000000000"){
        IDcontrolSignal = "000000000";
    }
    Rs = binaryToDecimal(instruction.substr(6, 5));
    Rt = binaryToDecimal(instruction.substr(11, 5));
    Rd = binaryToDecimal(instruction.substr(16, 5));
    readData1 = Register[Rs];
    readData2 = Register[Rt];
    sign_ext = instruction.substr(16, 16);
}

void execution(){
    writeData = readData2;
    EXRt = Rt;
    if(IDcontrolSignal[0] == '0'){ //RegDst
        EXRtRd = Rt;
    }
    else{
        EXRtRd = Rd;
    }
    forwardingUnit();
    ALU();
    EXcontrolSignal = IDcontrolSignal.substr(4, 5);
}

void memory(){
    MEMRtRd = EXRtRd;
    MEMALUout = EXALUout;
    if(EXcontrolSignal[1] == '1'){   //MEMread
        readData = dataMemory[MEMALUout/4];
    }
    else{
        readData = 0;
    }
    if(EXcontrolSignal[2] == '1'){   //MEMwrite
        dataMemory[MEMALUout/4] = writeData;
    }
    MEMcontrolSignal = EXcontrolSignal.substr(3, 2);
}

void writeBack(){
    if(MEMcontrolSignal[0] == '1'){  //RegWrite
        if(MEMcontrolSignal[1] == '1'){   //MemToReg
            Register[MEMRtRd] = readData;
            backData = readData;
        }
        else{
            Register[MEMRtRd] = MEMALUout;
            backData = MEMALUout;
        }
    }
    WBRtRd = MEMRtRd;
    WBcontrolSignal = MEMcontrolSignal;
}

void general(){
    if(MEM == true){
        WB = true;
    }
    else{
        WB = false;
    }
    if(EX == true){
        MEM = true;
    }
    else{
        MEM = false;
    }
    if(ID == true){
        EX = true;
    }
    else{
        EX = false;
    }
    if(IF == true){
        ID = true;
    }
    else{
        ID = false;
    }
    if(allInstructions.size() == 0){
        IF = false;
    }
    else{
        IF = true;
    }
}

void instructionMemory(string opCode){
    if(opCode == "000000"){       //R-type
        IDcontrolSignal = "110000010";
    }
    else if(opCode == "100011"){  //lw
        IDcontrolSignal = "000101011";
    }
    else if(opCode == "101011"){  //sw
        IDcontrolSignal = "000100100";
    }
    else if(opCode == "001000"){  //addi
        IDcontrolSignal = "000100010";
    }
    else if(opCode == "001100"){  //andi
        IDcontrolSignal = "011100010";
    }
    else if(opCode == "000100"){  //beq
        IDcontrolSignal = "001010000";
    }
}

void branchHazardDetection(){
    if(IDcontrolSignal[4] == '1' && readData1 == readData2){ //branch
        int temp = binaryToDecimal(sign_ext);
        for(int i=temp;i>1;i--){
            allInstructions.pop();
        }
        temp = temp * 4;
        brancNum = temp;
        instruction = "00000000000000000000000000000000";
    }
}

void hazardDetectionUnit(){
    if(EXcontrolSignal[1] == '1' && (EXRt == Rs || EXRt == Rt)){
        IF = false;
        ID = false;
        EX = false;
        MEM = true;
        WB = true;
        hazardTriggered = true;
    }
}

void forwardingUnit(){
    if(WBcontrolSignal[0] == '1'){
        if(Rs == WBRtRd && WBRtRd != 0 && Rs != MEMRtRd){
            readData1 = backData;
        }
        if(Rt == WBRtRd && WBRtRd != 0 && Rt != MEMRtRd){
            readData2 = backData;
        }
    }
    if(MEMcontrolSignal == "10"){
        if(Rs == MEMRtRd && MEMRtRd != 0){
            readData1 = MEMALUout;
        }
        if(Rt == MEMRtRd && MEMRtRd != 0){
            readData2 = MEMALUout;
        }
    }
}

void ALU(){
    string control = ALUcontrol();
    if(control == "error"){
        cout << "error" << endl;
    }

    int Data1 = readData1;
    int Data2 = readData2;
    if(IDcontrolSignal[3] == '1'){  //ALUSrc
        Data2 = binaryToDecimal(sign_ext);
    }

    if(control == "add"){
        EXALUout = Data1 + Data2;
    }
    else if(control == "sub"){
        EXALUout = Data1 - Data2;
    }
    else if(control == "and"){
        EXALUout = AND(decimalToBinary(Data1), decimalToBinary(Data2));
    }
    else if(control == "or"){
        EXALUout = OR(decimalToBinary(Data1), decimalToBinary(Data2));
    }
    else if(control == "slt"){
        if(readData1 < readData2){
            EXALUout = 1;
        }
        else{
            EXALUout = 0;
        }
    }
}

string ALUcontrol(){
    string ALUop = IDcontrolSignal.substr(1, 2);
    if(ALUop == "11"){
        return "and";
    }
    else if(ALUop == "00"){
        return "add";
    }
    else if(ALUop == "01"){
        return "sub";
    }
    else{
        string func = sign_ext.substr(10, 6);
        if(func == "100000"){
            return "add";
        }
        else if(func == "100010"){
            return "sub";
        }
        else if(func == "100100"){
            return "and";
        }
        else if(func == "100101"){
            return "or";
        }
        else if(func == "101010"){
            return "slt";
        }
    }

    return "error";
}

int AND(string data1, string data2){
    int num1 = data1.length();
    int num2 = data2.length();
    int length = 0;
    if(num2 > num1){
        for(int i=num2-num1;i>0;i--){
            data1 = '0' + data1;
        }
        length = num2;
    }
    else{
        for(int i=num1-num2;i>0;i--){
            data2 = '0' + data2;
        }
        length = num1;
    }
    string temp = "";
    for(int i=0;i<length;i++){
        if(data1[i] == '1' && data2[i] == '1'){
            temp = temp + '1';
        }
        else{
            temp = temp + '0';
        }
    }

    return binaryToDecimal(temp);
}

int OR(string data1, string data2){
    int num1 = data1.length();
    int num2 = data2.length();
    int length = 0;
    if(num2 > num1){
        for(int i=num2-num1;i>0;i--){
            data1 = '0' + data1;
        }
        length = num2;
    }
    else{
        for(int i=num1-num2;i>0;i--){
            data2 = '0' + data2;
        }
        length = num1;
    }
    string temp = "";
    for(int i=0;i<length;i++){
        if(data1[i] == '0' && data2[i] == '0'){
            temp = temp + '0';
        }
        else{
            temp = temp + '1';
        }
    }

    return binaryToDecimal(temp);
}

int binaryToDecimal(string str){
    int num = str.length();
    int temp = 0;
    int power = 1;
    for(int i=num-1;i>=0;i--){
        temp = temp + power*characterToInteger(str[i]);
        power = power * 2;
    }

    return temp;
}

string decimalToBinary(int number){
    string temp = "";
    while(number > 0){
        temp = IntegerToChacacter(number%2) + temp;
        number = number / 2;
    }

    return temp;
}

char IntegerToChacacter(int num){
    char temp;
    switch(num){
    case 0:
        temp = '0';
        break;
    case 1:
        temp = '1';
        break;
    case 2:
        temp = '2';
        break;
    case 3:
        temp = '3';
        break;
    case 4:
        temp = '4';
        break;
    case 5:
        temp = '5';
        break;
    case 6:
        temp = '6';
        break;
    case 7:
        temp = '7';
        break;
    case 8:
        temp = '8';
        break;
    case 9:
        temp = '9';
        break;
    }

    return temp;
}

int characterToInteger(char ch){
    int temp = 0;
    switch(ch){
    case '0':
        temp = 0;
        break;
    case '1':
        temp = 1;
        break;
    case '2':
        temp = 2;
        break;
    case '3':
        temp = 3;
        break;
    case '4':
        temp = 4;
        break;
    case '5':
        temp = 5;
        break;
    case '6':
        temp = 6;
        break;
    case '7':
        temp = 7;
        break;
    case '8':
        temp = 8;
        break;
    case '9':
        temp = 9;
        break;
    }

    return temp;
}

void print(int clockCycle, int i){
    fstream Fout;
    switch(i){
    case 0:
        Fout.open("genResult.txt", ios::out|ios::app);
        break;
    case 1:
        Fout.open("dataResult.txt", ios::out|ios::app);
        break;
    case 2:
        Fout.open("loadResult.txt", ios::out|ios::app);
        break;
    case 3:
        Fout.open("branchResult.txt", ios::out|ios::app);
        break;
    }
    if(!Fout){
        cout << "Cannot open the file " << endl;
    }
    Fout << "clockCycle " << clockCycle << ":" << endl << endl;
    Fout << "Registers:" << endl;
    for(int i=0;i<10;i++){
        Fout << "$" << i << ": " << Register[i] << endl;
    }
    Fout << endl << "Data memory:" << endl;
    Fout << "0x00: " << dataMemory[0] << endl;
    Fout << "0x04: " << dataMemory[1] << endl;
    Fout << "0x08: " << dataMemory[2] << endl;
    Fout << "0x0C: " << dataMemory[3] << endl;
    Fout << "0x10: " << dataMemory[4] << endl << endl;

    Fout << "IF/ID: " << endl;
    Fout << "PC" << "\t" << "\t" << PC << endl;
    Fout << "instruction" << "\t" << instruction << endl << endl;

    Fout << "ID/EX: " << endl;
    Fout << "ReadData1" << "\t" << readData1 << endl;
    Fout << "ReadData2" << "\t" << readData2 << endl;
    Fout << "sign_ext" << "\t" << binaryToDecimal(sign_ext) << endl;
    Fout << "Rs" << "\t" << "\t" << Rs << endl;
    Fout << "Rt" << "\t" << "\t" << Rt << endl;
    Fout << "Rd" << "\t" << "\t" << Rd << endl;
    Fout << "Control signals" << "\t" << IDcontrolSignal << endl << endl;

    Fout << "EX/MEM: " << endl;
    Fout << "ALUout" << "\t" << "\t" << EXALUout << endl;
    Fout << "WriteData" << "\t" << writeData << endl;
    Fout << "Rt/Rd" << "\t" << "\t" << EXRtRd << endl;
    Fout << "Control signals" << "\t" << EXcontrolSignal << endl << endl;

    Fout << "MEM/WB: " << endl;
    Fout << "ReadData" << "\t" << readData << endl;
    Fout << "ALUout" << "\t" << "\t" << MEMALUout << endl;
    Fout << "Rt/Rd" << "\t" << "\t" << MEMRtRd << endl;
    Fout << "Control signals" << "\t" << MEMcontrolSignal << endl;
    Fout << "===================================================================" << endl;

    Fout.close();
}

void reset(){
    Register[0] = 0;
    Register[1] = 9;
    Register[2] = 5;
    Register[3] = 7;
    Register[4] = 1;
    Register[5] = 2;
    Register[6] = 3;
    Register[7] = 4;
    Register[8] = 5;
    Register[9] = 6;
    dataMemory[0] = 5;
    dataMemory[1] = 9;
    dataMemory[2] = 4;
    dataMemory[3] = 8;
    dataMemory[4] = 7;

    IF = true;
    ID = false;
    EX = false;
    MEM = false;
    WB = false;
    hazardTriggered = false;

    PC = 0;
    brancNum = 0;
    instruction = "00000000000000000000000000000000";

    readData1 = 0;
    readData2 = 0;
    sign_ext = "0000000000000000";
    Rs = 0;
    Rt = 0;
    Rd = 0;
    IDcontrolSignal = "000000000";

    EXALUout = 0;
    writeData = 0;
    EXRtRd = 0;
    EXRt = 0;
    EXcontrolSignal = "00000";

    readData = 0;
    MEMALUout = 0;
    MEMRtRd = 0;
    MEMcontrolSignal = "00";

    WBRtRd = 0;
    backData = 0;
    WBcontrolSignal = "00";
}
