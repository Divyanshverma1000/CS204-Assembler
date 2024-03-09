#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm> 
#include<bitset>
#include <sstream>
#include <iomanip>
#include <stdexcept>

using namespace std;


// My first iteration over the instructions i need to verify whether the instruction is valid or not before incrementing instruction index2 which is responsible
// for correct indexing of labels
// currently i am doing it using number of arguments
// improve it









map<string,int> label_map;
bool isvalid=false;
int instruction_index=0;
int instruction_index2=0;

string intToHex(int number) {
    stringstream stream;
    stream << hex << setw(8) << setfill('0') << number;
    return "0x"+stream.str();
}

string binaryToHex(string binary) {
    // Convert binary string to integer
    bitset<32> bits(binary);
    unsigned long n = bits.to_ulong();

    // Convert integer to hexadecimal string with 8 characters and prefixed with "0x"
    stringstream ss;
    ss << "0x" << uppercase << setfill('0') << setw(8) << hex << n;
    string hex = ss.str();

    return hex;
}



vector<string> parser(const string& instruction) {
    vector<string> info;
    
    string cleanedInstruction = instruction.substr(0, instruction.find("#")); // Remove comments
    if(cleanedInstruction.empty()) return info;
    stringstream ss(cleanedInstruction);
    string temp_word;
    // cout<<cleanedInstruction<<endl;

    while (getline(ss, temp_word, ' ')) { // Tokenize by space
        // cout<<"some"<<endl;
        // cout<<temp_word<<endl;
        if (temp_word.empty()) continue; // Skip empty tokens
        size_t col_start = temp_word.find(':');
        
        // when there is just label and no instruction in same line it gets handled here
        if(col_start!=string::npos){
            string label_part=temp_word.substr(0,col_start+1);   //error handled here for wrond bta address
            label_map[label_part]={instruction_index2};
            //  instruction_index++; will thinkk about it
            info.push_back(temp_word);
            continue; 
        }
   
        size_t imm_start = temp_word.find('(');
        size_t imm_end = temp_word.find(')');

        if (imm_start != string::npos && imm_end != string::npos && imm_start < imm_end) {
            // S-format instruction: extract imm and rs2 from something like "-4(x11)"
            string imm = temp_word.substr(0, imm_start);
            string rs2 = temp_word.substr(imm_start + 1, imm_end - imm_start - 1);
            if (!imm.empty()) info.push_back(imm); // Adding immediate value #give syntax error here
            if (!rs2.empty()) info.push_back(rs2); // Adding rs2
        } else {
            // Handle comma as a secondary delimiter within tokens
            stringstream tokenStream(temp_word);
            string subToken;
            while (getline(tokenStream, subToken, ',')) {
                if (!subToken.empty()) info.push_back(subToken); // Adding operands and opcode without commas
            }
        }
    }
    
    // when there is instructions and label on same line it gets handled here
    if(info.size()>1&& info[0].find(':')!=string::npos) {
             
             int end=info[0].length();
             int col_start=info[0].find(':');

             if(info[0].length()!=col_start+1){
                     info[0]=info[0].substr(col_start+1,end-col_start+1);
                    //  cout<<info[0];
             }
             else{
             info.erase(info.begin());
             }
        }
    
    if(info.size()>1) isvalid=true;
   
    // cout<<"it leaves"<<endl;
    return info;
}


string registerToBinary(string reg) {
    int reg_num = stoi(reg.substr(1)); // Extract the number part and convert to integer
    string binary = bitset<5>(reg_num).to_string(); // Convert to 5-bit binary representation
    return binary;
}

string immediateToBinary(const string& immediate, int bits) {
    // Convert immediate string to integer
    int immediateValue = stoi(immediate);

    // Check if the immediate value is within the valid range for the number of bits
    if (immediateValue < -(1 << (bits - 1)) || immediateValue >= (1 << (bits - 1))) {
        throw invalid_argument("Immediate value out of range for given number of bits.");
    }

    // Convert the immediate value to its binary representation
    bitset<32> bit(immediateValue & ((1 << bits) - 1)); // Mask out the higher bits
    return bit.to_string().substr(32 - bits);
}


int main() {
    vector<string> R_operations = {
        "add", "and", "or", "sll", "slt", "sra", "srl", "sub", "xor", "mul", "div", "rem"
    };
    vector<string> I_operations = {
        "addi", "andi", "ori", "lb", "ld", "lh", "lw", "jalr"
    };

    vector<string> S_operations = {
        "sb", "sw", "sd", "sh"
    };

    vector<string> SB_operations = {
        "beq", "bne", "bge", "blt"
    };

    vector<string> U_operations = {
        "auipc", "lui"
    };

    vector<string> UJ_operations = {
        "jal"
    };


    map<string, vector<string>> R_map;

    // Fill the map with opcode, funct3, and funct7 values for each R operation
    R_map["add"] = {"0110011", "000", "0000000"};
    R_map["and"] = {"0110011", "111", "0000000"};
    R_map["or"]  = {"0110011", "110", "0000000"};
    R_map["sll"] = {"0110011", "001", "0000000"};
    R_map["slt"] = {"0110011", "010", "0000000"};
    R_map["sra"] = {"0110011", "101", "0100000"};
    R_map["srl"] = {"0110011", "101", "0000000"};
    R_map["sub"] = {"0110011", "000", "0100000"};
    R_map["xor"] = {"0110011", "100", "0000000"};
    R_map["mul"] = {"0110011", "000", "0000001"};
    R_map["div"] = {"0110011", "100", "0000001"};
    R_map["rem"] = {"0110011", "110", "0000001"};


map<string, vector<string>> I_map;

    // Fill the map with opcode and funct3 values for each I operation
    I_map["addi"] = {"0010011", "000"};
    I_map["andi"] = {"0010011", "111"};
    I_map["ori"] = {"0010011", "110"};
    I_map["lb"] = {"0000011", "000"};
    I_map["ld"] = {"0000011", "011"};
    I_map["lh"] = {"0000011", "001"};
    I_map["lw"] = {"0000011", "010"};
    I_map["jalr"] = {"1100111", "000"};
    
    map<string, vector<string>> S_map;

    S_map["sb"] = {"0100011", "000"};
    S_map["sw"] = {"0100011", "010"};
    S_map["sd"] = {"0100011", "011"};
    S_map["sh"] = {"0100011", "001"};

   map<string, vector<string>> SB_map;

   SB_map["beq"] = {"1100011", "000"};
   SB_map["bne"] = {"1100011", "001"};
   SB_map["bge"] = {"1100011", "101"};
   SB_map["blt"] = {"1100011", "100"};

   map<string, vector<string>> U_map;

   U_map["auipc"] = {"0010111"};
   U_map["lui"] = {"0110111"};
  
   map<string, vector<string>> UJ_map;

   UJ_map["jal"] = {"1101111"};


    // File handling
    string text;
    ifstream Myfile("code.txt");
    vector<string> instruction_set;

    while(getline(Myfile,text)){
        vector<string> parsed_instruction1 = parser(text);
        instruction_set.push_back(text);
        if(isvalid) instruction_index2++;  
        isvalid=false;  
        // cout<<instruction_index2<<endl;
    }


  
    // for (const auto& pair : label_map) {
    //     std::cout << "Key: " << pair.first << ", Value: " << pair.second << std::endl;
    // }
int i=0;
// cout<<"Here once"<<endl;
  // Seek to the beginning of the file
Myfile.clear();  // Clear any error flags
Myfile.seekg(0);  // Seek to the beginning of the file
    //    cout<<"it works"<<endl;
    // Getting asm code in vector
    for(string text:instruction_set) {
        // cout<<"it enters :"<<endl
        if(text.empty()) continue;
        // cout<<"iteration nu: "<<i<<endl;
        // cout<<text<<endl; 
        // cout<<"index"<<endl;
        vector<string> parsed_instruction = parser(text);
        // cout<<"returned"<<endl;
        // cout<<"Text to be parsed  :  "<<text<<endl;
        // cout<<"it is here"<<endl;
        // cout<<parsed_instruction[0]<<endl;
    
        string operation = parsed_instruction[0];
        // cout<<operation<<endl;
        if (find(R_operations.begin(), R_operations.end(), operation) != R_operations.end()) {
            vector<string> machine_code_vec = R_map[operation];
            string machine_code_R;
            string opcode =machine_code_vec[0];
            string func3 =machine_code_vec[1];
            string func7 =machine_code_vec[2];
            string rd = registerToBinary(parsed_instruction[1]);
            string rs1 = registerToBinary(parsed_instruction[2]);
            string rs2 = registerToBinary(parsed_instruction[3]);

            machine_code_R = func7 +rs2+ rs1+ func3+ rd+ opcode;
            string pc=intToHex(instruction_index*4);

            cout <<pc<<" "<<text<< ": " << binaryToHex(machine_code_R) << endl; 
            // cout<<"ins index at R: "<<instruction_index<<endl;
            instruction_index++;
            // cout<<"Entered here"<<endl;
        }else if (find(I_operations.begin(), I_operations.end(), operation) != I_operations.end()) {
            vector<string> machine_code_vec = I_map[operation];
            string machine_code_I;
            string opcode =machine_code_vec[0];
            string func3 =machine_code_vec[1];
            string rd = registerToBinary(parsed_instruction[1]);
            string rs1 = registerToBinary(parsed_instruction[2]);
            string imm = immediateToBinary(parsed_instruction[3],12);

            machine_code_I = imm+ rs1+ func3+ rd+ opcode;
            string pc=intToHex(instruction_index*4);

            cout <<pc<<" "<<text<< ": " << binaryToHex(machine_code_I) << endl;  //Venus is giving wrong machine code for this instruction
            instruction_index++;
        } else if (find(S_operations.begin(), S_operations.end(), operation) != S_operations.end()) {
            vector<string> machine_code_vec = S_map[operation];
            string machine_code_S;
            string opcode =machine_code_vec[0];
            string func3 =machine_code_vec[1];
            string rs1 = registerToBinary(parsed_instruction[1]);
            string rs2 = registerToBinary(parsed_instruction[3]);
            string imm = immediateToBinary(parsed_instruction[2],12);
            // cout<<"imm values: "<<imm<<endl;
            string imm7=imm.substr(0,7);

            // cout<<"imm7 values: "<<imm7<<endl;

            string imm5 = imm.substr(7,5);
            // cout<<"imm5 values: "<<imm5<<endl;
            // cout<<"imm5 :"<<binaryToHex(imm5)<<endl;                                      #some doubt heres

            machine_code_S = imm7 + rs2+ rs1+ func3+ imm5+ opcode;
            string pc=intToHex(instruction_index*4);

            cout <<pc<<" "<<text<< ": " << binaryToHex(machine_code_S) << endl;
            instruction_index++;
        } else if (find(SB_operations.begin(), SB_operations.end(), operation) != SB_operations.end()) {//Venus is giving wrong machine code for this instruction
            vector<string> machine_code_vec = SB_map[operation];
            string machine_code_SB;
            string opcode =machine_code_vec[0];
            string func3 =machine_code_vec[1];
            string rs1 = registerToBinary(parsed_instruction[1]);
            string rs2 = registerToBinary(parsed_instruction[2]);
            string key=parsed_instruction[3]+":";
            int newimm=(label_map[key]-instruction_index)*2;
            // cout<<"label: "<<parsed_instruction[3]<<endl;
            // cout<<"key : "<<key<<endl;
            // cout<<"label_index: "<<label_map[key]<<endl;
            // cout<<"instruction index: "<<instruction_index<<endl;
            // cout<<"New imm: "<<newimm<<endl;
            string imm1=to_string(newimm);
            string imm = immediateToBinary(imm1,12);
            // cout<<"imm values: "<<imm<<endl;
            char imm12=imm[0];
            char imm11=imm[1];
            string imm10_5 = imm.substr(2,6);
            string imm4_1 = imm.substr(8,4);

            machine_code_SB = imm12+imm10_5+ rs2+ rs1+ func3+imm4_1+imm11+ opcode;
            // cout<<"Machine code: "<<machine_code_SB<<endl;
            string pc=intToHex(instruction_index*4);

            cout <<pc<<" "<<text<< ": " << binaryToHex(machine_code_SB) << endl;
            instruction_index++;
        } else if (find(U_operations.begin(), U_operations.end(), operation) != U_operations.end()) {
            vector<string> machine_code_vec = U_map[operation];
            string machine_code_U;
            string opcode =machine_code_vec[0];
            // string func3 =machine_code_vec[1];
            string rd = registerToBinary(parsed_instruction[1]);
            string imm20 = immediateToBinary(parsed_instruction[2],20);

            machine_code_U = imm20+ rd+ opcode;
            string pc=intToHex(instruction_index*4);

            cout <<pc<<" "<<text<< ": " << binaryToHex(machine_code_U) << endl;
            instruction_index++;
        } else if (find(UJ_operations.begin(), UJ_operations.end(), operation) != UJ_operations.end()) {
            vector<string> machine_code_vec = UJ_map[operation];
            string machine_code_UJ;
            string opcode =machine_code_vec[0];
            // string func3 =machine_code_vec[1];
            string rd = registerToBinary(parsed_instruction[1]);
            string key=parsed_instruction[2]+":";

            int newimm=(label_map[key]-instruction_index)*2;
            
            // cout<<"instruction index: "<<instruction_index<<endl;
            // cout<<"label index: "<<label_map[key]<<endl;

            // cout<<"NEw imm: "<<newimm*2<<endl;
            string imm1=to_string(newimm);
            string imm20temp = immediateToBinary(imm1,20);
            string imm20=imm20temp[0]+imm20temp.substr(10,10)+imm20temp[10]+imm20temp.substr(1,8);
            machine_code_UJ = imm20+ rd+ opcode;
            string pc=intToHex(instruction_index*4);
            
            cout <<pc<<" "<<text<< ": " <<binaryToHex(machine_code_UJ) << endl;
            instruction_index++;
        } else {
            // cout << "Unknown instruction: " <<instruction_index<<" "<<text<< endl;
            // cout<<"after return as there is no op"<<endl;
            continue;
        }
        i++;
        // cout<<"Instruction index : "<<instruction_index<<endl;
    } 

    return 0;
}
