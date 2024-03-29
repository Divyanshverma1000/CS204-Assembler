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

bool endsWithColon(const std::string& str) {
    if (str.empty()) {
        return false; // Empty string cannot end with a colon
    }
    char lastChar = str.back();
    return lastChar == ':';
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
    stringstream ss(cleanedInstruction);
    string temp_word;

    // // Skip empty lines
    // if (cleanedInstruction.find_first_not_of(" \t\n\v\f\r") == string::npos) {
    //     return info; // Empty line, return empty vector
    // }

    while (getline(ss, temp_word, ' ')) { // Tokenize by space
        if (temp_word.empty()) continue; // Skip empty tokens

        size_t imm_start = temp_word.find('(');
        size_t imm_end = temp_word.find(')');

        if (imm_start != string::npos && imm_end != string::npos && imm_start < imm_end) {
            // S-format instruction: extract imm and rs2 from something like "-4(x11)"
            string imm = temp_word.substr(0, imm_start);
            string rs2 = temp_word.substr(imm_start + 1, imm_end - imm_start - 1);
            if (!imm.empty()) info.push_back(imm); // Adding immediate value
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
    string segment_type;
    ifstream Myfile("code.txt");
    int instruction_idx=0;
    map<vector<string>, int> instructionPCMap;
    vector<vector<string>> data_segment;
    // Getting asm code in vector
    while (getline(Myfile, text)) {

        if (text.find(".text") != string::npos) {
            segment_type = "text";
            continue;
        } else if (text.find(".data") != string::npos) {
            segment_type = "data";
            continue;
        }
        // } else if (segment_type.empty()) {
        //     continue; // Skip lines before encountering any segment type
        // }

        if (segment_type == "text") {
             vector<string> parsed_instruction = parser(text);
        if (parsed_instruction.empty()) {
        continue; // Skip empty lines
       }
             instructionPCMap[parsed_instruction]=instruction_idx; // this helps to find the instruction corresponding to the pc  in jump type instr
             string operation = parsed_instruction[0];
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
            cout << "0x"<<hex <<instruction_idx*4<<" "<< text << ": " << binaryToHex(machine_code_R) << endl; 
            instruction_idx++;

            // cout << "Machine Code for " << text << ": " << binaryToHex(machine_code_R) << endl; 
        }else if (find(I_operations.begin(), I_operations.end(), operation) != I_operations.end()) {
            vector<string> machine_code_vec = I_map[operation];
            string machine_code_I;
            string opcode =machine_code_vec[0];
            string func3 =machine_code_vec[1];
            string rd = registerToBinary(parsed_instruction[1]);
            string rs1 = registerToBinary(parsed_instruction[2]);
            string imm = immediateToBinary(parsed_instruction[3],12);

            machine_code_I = imm+ rs1+ func3+ rd+ opcode;

            cout << "0x"<<hex <<instruction_idx*4<<" "<< text << ": " << binaryToHex(machine_code_I) << endl; 
            instruction_idx++;

        } else if (find(S_operations.begin(), S_operations.end(), operation) != S_operations.end()) {
            vector<string> machine_code_vec = S_map[operation];
            string machine_code_S;
            string opcode =machine_code_vec[0];
            string func3 =machine_code_vec[1];
            string rs1 = registerToBinary(parsed_instruction[1]);
            string rs2 = registerToBinary(parsed_instruction[3]);
            string imm = immediateToBinary(parsed_instruction[2],12);
            string imm7 = imm.substr(0,7);
            string imm5 = imm.substr(7,5);
            

            machine_code_S = imm7+ rs2+ rs1+ func3+ imm5+ opcode;

            cout << "0x"<<hex <<instruction_idx*4<< " "<<text << ": " << binaryToHex(machine_code_S) << endl; 
            instruction_idx++;

        } else if (find(SB_operations.begin(), SB_operations.end(), operation) != SB_operations.end()) {
            vector<string> machine_code_vec = SB_map[operation];
            string machine_code_SB;
            string opcode =machine_code_vec[0];
            string func3 =machine_code_vec[1];
            string rs1 = registerToBinary(parsed_instruction[1]);
            string rs2 = registerToBinary(parsed_instruction[2]);
            string imm = immediateToBinary(parsed_instruction[3],12);
            string imm7 = imm.substr(0,7);
            string imm5 = imm.substr(7,5);

            machine_code_SB = imm7+ rs2+ rs1+ func3+ imm5+ opcode;
            cout << "0x"<<hex <<instruction_idx*4<<" "<< text << ": " << binaryToHex(machine_code_SB) << endl; 
            instruction_idx++;

        } else if (find(U_operations.begin(), U_operations.end(), operation) != U_operations.end()) {
            vector<string> machine_code_vec = U_map[operation];
            string machine_code_U;
            string opcode =machine_code_vec[0];
            // string func3 =machine_code_vec[1];
            string rd = registerToBinary(parsed_instruction[1]);
            string imm20 = immediateToBinary(parsed_instruction[2],20);

            machine_code_U = imm20+ rd+ opcode;

            cout << "0x"<<hex <<instruction_idx*4<<" "<< text << ": " << binaryToHex(machine_code_U) << endl; 
            instruction_idx++;

        } else if (find(UJ_operations.begin(), UJ_operations.end(), operation) != UJ_operations.end()) {
            vector<string> machine_code_vec = UJ_map[operation];
            string machine_code_UJ;
            string opcode =machine_code_vec[0];
            // string func3 =machine_code_vec[1];
            string rd = registerToBinary(parsed_instruction[1]);
            string imm20 = immediateToBinary(parsed_instruction[2],20);

            machine_code_UJ = imm20+ rd+ opcode;

            cout << "0x"<<hex <<instruction_idx*4<<" "<< text << ": " << binaryToHex(machine_code_UJ) << endl; 
            instruction_idx++;

        }
        //  else if(operation==".text"){

        // }else if(operation==".data"){

        // }
        else if(endsWithColon(operation)){

        }else {
            cout << "Unknown instruction: " << text << endl;
        }
            // text_segment.push_back(text);
        } else if (segment_type == "data") {
            // data_segment.push_back(text);
            data_segment.push_back(parser(text));
        }
    
       
       
    }

    return 0;
}
