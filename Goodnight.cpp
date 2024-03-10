#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <algorithm> 
#include<bitset>
#include <sstream>
#include <iomanip>
#include <stdexcept>

// included necessary libraries to work with

using namespace std;



map<string,int> label_map;           //map which maps labels to their instruction number in which they appear or line number
bool isvalid=false;                  //used later to  determine whether instruction is valid or not
int instruction_index=0;             //it is instruction no counter for second iteration of  all instructions
int instruction_index2=0;            //it is instruction no counter for first iteration  of  all instructions
bool is_iteration2_active=false;


//div
vector<string> instruction;
vector<string> data_segment;
// unordered_map<string, string> label_map; // Mapping labels to instructions
int i=0;
//div



vector<string> all_operations = {"add", "and", "or", "sll", "slt", "sra", "srl", "sub", "xor", "mul", "div", "rem",
                                         "addi", "andi", "ori", "lb", "ld", "lh", "lw", "jalr",
                                         "sb", "sw", "sd", "sh",
                                         "beq", "bne", "bge", "blt",
                                         "auipc", "lui",
                                         "jal"
                                         };














//integer to hexadecimal  (used for getting pc in hexadecimal)
string intToHex(int number) {
    stringstream stream;
    stream << hex << number;
    return "0x" + stream.str();
}


//hex to int used in U format
int hexToInt(const string& hexString) {
    int result;
    stringstream ss;
    ss << hex << hexString;
    ss >> result;
    return result;
}


//convert binary string to hexadecimal (this is for 32 bit machine code)
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


bool isNumber(const string& str) {
    try {
        int num = stoi(str);
        // If stoi succeeds, str is a number
        return true;
    } catch (const invalid_argument& ia) {
        // stoi could not convert the string to a number
        return false;
    } catch (const out_of_range& oor) {
        // stoi converted the string to a number, but the result is out of range
        return false;
    }
}









//Parser function which returns useful fields to the main in vector
//for eg add x1,x2,x3   parser will return [add x1 x2 x3] in vector 
vector<string> parser(const string& instruction,vector<vector<uint8_t>>& memory) {
    vector<string> info;
    
    string cleanedInstruction = instruction.substr(0, instruction.find("#")); // Remove comments
    if(cleanedInstruction.empty()) return info;

    // Iterate over the string and remove extra spaces before ":"
    // for (size_t i = 0; i < cleanedInstruction.length(); i++) {
    //     if (cleanedInstruction[i] == ':' && i > 0 && cleanedInstruction[i - 1] == ' ') {
    //         cleanedInstruction.erase(i - 1, 1); // Remove the extra space
    //     }
    // }
     for(int i=0;i<cleanedInstruction.length();i++){
        if(cleanedInstruction[i]==',') cleanedInstruction[i]=' ';
     }

    stringstream ss(cleanedInstruction);
    string temp_word;

    

    while (getline(ss, temp_word, ' ')) { // Tokenize by space    
        
        //here tempword contains the token separated by space  and it collects them one by one in loop

        if (temp_word.empty()) continue; // Skip empty tokens
        
        
        size_t col_start = temp_word.find(':');  // gives position of colon ':'
        
        // when there is just label and no instruction in same line it gets handled here
        /*eg    for:
                add x1 x2 x3 
         */

        if(col_start!=string::npos){  //check whether ':' colon was there or not in tempword
            //when there isn't any space between : and operation it label needs to be separated and it is done here
            //eg label:add x1 x2 x3
            string label_part=temp_word.substr(0,col_start+1);   //error handled here for wrong bta address
            if(!is_iteration2_active){
            label_map[label_part]={instruction_index2};          //here mapping the label with instruction index
            // cout<<"Label index stored: "<<instruction_index2<<endl;
            }
                    
            info.push_back(temp_word);                           //even if it is combined word like for:add   it will be pushed it will be dealt 
                                                                 //at the end of loop

            // cout<<label_part<<"\t"<<instruction_index2<<endl;                                                    
            continue; //no need to go below
        }
        
    
    if(!is_iteration2_active){
        //div code
        
        // Check for data segment lines
        if (temp_word == ".byte") {
            while (getline(ss, temp_word, ' ')) {
                if (!temp_word.empty()) {
                    vector<uint8_t> dataRow;
                    // cout<<"going to push"<<endl;
                    dataRow.push_back(stoi(temp_word));
                    // cout<<"done pushing"<<endl;
                    i++;
                    // cout<<"i incremented"<<endl;
                    memory.push_back(dataRow);
                }
            }
            return info;
        } else if (temp_word == ".half") {
            while (getline(ss, temp_word, ',')) {
                if (!temp_word.empty()) {
                    vector<uint8_t> dataRow;
                    int value = stoi(temp_word);
                    dataRow.push_back((value >> 8) & 0xFF); // Most significant byte
                    dataRow.push_back(value & 0xFF);        // Least significant byte
                    i++;
                    memory.push_back(dataRow);
                }
            }
            return info;
        } else if (temp_word == ".asciiz") {
            // Store each character as a byte
            string str;
            if (ss >> str) {
                vector<uint8_t> dataRow1;
                for (char c : str) {
                    if(c=='\"'){continue;}
                    vector<uint8_t> dataRow;
                    dataRow.push_back(static_cast<uint8_t>(c));
                    memory.push_back(dataRow);  
                }
                // Null-terminate the string
                dataRow1.push_back(0);
                i++;
                memory.push_back(dataRow1);
            }
            return info;
        } else if (temp_word == ".word") {
            while (getline(ss, temp_word, ' ')) {
                if (!temp_word.empty()) {
                    vector<uint8_t> dataRow;
                    int value = stoi(temp_word);
                    dataRow.push_back((value >> 24) & 0xFF); // Most significant byte
                    dataRow.push_back((value >> 16) & 0xFF);
                    dataRow.push_back((value >> 8) & 0xFF);
                    dataRow.push_back(value & 0xFF);        // Least significant byte
                    i++;
                    memory.push_back(dataRow);
                }
            }
            return info;
        } else if (temp_word == ".dword") {
            // Store each number in 8 bytes
            while (getline(ss, temp_word, ',')) {
                if (!temp_word.empty()) {
                    vector<uint8_t> dataRow;
                    int64_t value = stoll(temp_word);
                    for (int it = 7; it >= 0; --it) {
                        dataRow.push_back((value >> (it * 8)) & 0xFF);
                    }
                    i++;
                    memory.push_back(dataRow);
                }
            }
            return info;
        }


        //div code


    }















    
        //for load store instructions stores the indices of opening and closing parenthesis
        size_t imm_start = temp_word.find('(');
        size_t imm_end = temp_word.find(')');


       //condition says that if both opening and closing parenthesis are there and opening comes before closing then go inside if
       //indirectly saying it is load store instruction
        if (imm_start != string::npos && imm_end != string::npos && imm_start < imm_end) {
            // S-format instruction: extract imm and rs2 from something like "-4(x11)"
            //eg format is : lw x1 imm(rs2)
            string imm = temp_word.substr(0, imm_start);
            string rs2 = temp_word.substr(imm_start + 1, imm_end - imm_start - 1);
            if (!imm.empty()) info.push_back(imm); // Adding immediate value #give syntax error here
            if (!rs2.empty()) info.push_back(rs2); // Adding rs2
        } else {
            //it enters in the else part to remove if delimiter is comma  eg add x1,x2,x3
            // Handle comma as a secondary delimiter within tokens
            stringstream tokenStream(temp_word);
            string subToken;
            while (getline(tokenStream, subToken, ',')) {
                if (!subToken.empty()) info.push_back(subToken); // Adding operands and opcode without commas
            }
        }
        
    }
    
    // when there is instructions and label on same line it gets handled here
    //here we extract the label: part  from label:operation ( when there is no space between colon and operation)
    if(info.size()>1&& info[0].find(':')!=string::npos) {
             
             int end=info[0].length();
             int col_start=info[0].find(':');

             if(info[0].length()!=col_start+1){   //it enters in if in actual there is no space between colon and operation
                     info[0]=info[0].substr(col_start+1,end-col_start+1);
                    //  cout<<info[0];
             }
             else{                               //it enters else part it there is space between colon and operation
             info.erase(info.begin());
             }
        }
    


    if(info.size()>1 && find(all_operations.begin(),all_operations.end(),info[0])!=all_operations.end()) isvalid=true;              //weak condition to check if instruction is valid or not
   
    return info;
}


//converts register to binary  eg x1 will becomes 00001 in binary
string registerToBinary(string reg) {
    int reg_num = stoi(reg.substr(1)); // Extract the number part and convert to integer
    string binary = bitset<5>(reg_num).to_string(); // Convert to 5-bit binary representation
    return binary;
}


//converts immediate to binary (signed representation)           IMP-NOTE:it may give error in I format case as it required unsigned representation of immediate

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






//main starts here



int main() {

    vector<vector<uint8_t>> memory;

    //stores all the operatoions in corresponding format vector

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

    //Here we map eachh operation with its corresponding opcode funct3 and funct7

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

   //First iteration where we will map each label with its corresponding instruction index


    while(getline(Myfile,text)){
        vector<string> parsed_instruction1 = parser(text,memory);
        instruction_set.push_back(text);
        if(isvalid){ instruction_index2++; }
        isvalid=false;  
    }
    // for (const auto& pair : label_map) {
    //     std::cout << "Key: " << pair.first << ", Value: " << pair.second << std::endl;
    // }

is_iteration2_active=true;
int i=0;

// Seek to the beginning of the file

Myfile.clear();  // Clear any error flags
Myfile.seekg(0);  // Seek to the beginning of the file



    // Getting asm code in vector
    //traversing each instruction in instruction set again (Second interation)
    for(string text:instruction_set) {
        
        if(text.empty()) continue;
        vector<string> parsed_instruction = parser(text,memory);
        if(parsed_instruction.empty()) continue;
    
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
            string pc=intToHex(instruction_index*4);

            cout <<pc<<" "<<text<< ": " << binaryToHex(machine_code_R) << endl; 
            instruction_index++;
            
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

            cout <<pc<<" "<<text<< ": " << binaryToHex(machine_code_I) << endl; 
            instruction_index++;
        } else if (find(S_operations.begin(), S_operations.end(), operation) != S_operations.end()) {
            vector<string> machine_code_vec = S_map[operation];
            string machine_code_S;
            string opcode =machine_code_vec[0];
            string func3 =machine_code_vec[1];
            string rs2 = registerToBinary(parsed_instruction[1]);
            string rs1 = registerToBinary(parsed_instruction[3]);
            string imm = immediateToBinary(parsed_instruction[2],12);
    
            string imm7=imm.substr(0,7);
            
            // cout<<"imm7 values: "<<imm7<<endl;

            string imm5 = imm.substr(7,5);
            // cout<<"imm5 values: "<<imm5<<endl;
            // cout<<"imm5 :"<<binaryToHex(imm5)<<endl;                                      

            machine_code_S = imm7 + rs2+ rs1+ func3+ imm5+ opcode;
            string pc=intToHex(instruction_index*4);

            cout <<pc<<" "<<text<< ": " << binaryToHex(machine_code_S) << endl;
            instruction_index++;
        } else if (find(SB_operations.begin(), SB_operations.end(), operation) != SB_operations.end()) {
            vector<string> machine_code_vec = SB_map[operation];
            string machine_code_SB;
            string opcode =machine_code_vec[0];
            string func3 =machine_code_vec[1];
            string rs1 = registerToBinary(parsed_instruction[1]);
            string rs2 = registerToBinary(parsed_instruction[2]);
            string key=parsed_instruction[3]+":";
            int newimm=(label_map[key]-instruction_index)*2;
            
            if(isNumber(parsed_instruction[3])){
                newimm=stoi(parsed_instruction[3])/2;
                // cout<<"yeees baby"<<endl;
            }
            


            // cout<<"label: "<<parsed_instruction[3]<<endl;
            // cout<<"key : "<<key<<endl;
            // cout<<"label_index: "<<label_map[key]<<endl;
            // cout<<"instruction index: "<<instruction_index<<endl;
            // cout<<"New imm: "<<newimm*2<<endl;
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
            string imm20;
            int imm1;
            //if immediate is in hexadecimal
            if(parsed_instruction[2].find("0x")!=string::npos){
                 imm1=hexToInt(parsed_instruction[2].substr(2));
                 imm20=immediateToBinary(to_string(imm1),20);
            }
            else{
                imm20=immediateToBinary(parsed_instruction[2],20);
            }
            




            // string imm20 = immediateToBinary(parsed_instruction[2],20);

            machine_code_U = imm20+ rd+ opcode;
            string pc=intToHex(instruction_index*4);

            cout <<pc<<" "<<text<< ":" << binaryToHex(machine_code_U) << endl;
            instruction_index++;
        } else if (find(UJ_operations.begin(), UJ_operations.end(), operation) != UJ_operations.end()) {
            vector<string> machine_code_vec = UJ_map[operation];
            string machine_code_UJ;
            string opcode =machine_code_vec[0];
            // string func3 =machine_code_vec[1];
            string rd = registerToBinary(parsed_instruction[1]);
            string key=parsed_instruction[2]+":";
            int newimm=(label_map[key]-instruction_index)*2;

            if(isNumber(parsed_instruction[2])){
                  newimm=stoi(parsed_instruction[2])/2;    
            }
         


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



//div
   cout<<"revOrder Stack"<<endl;
   cout<<"-------------------------------------"<<endl;
    unsigned int start_address = 0x10000000;
    reverse(memory.begin(), memory.end());
    int total=0;
     for (const auto& block : memory) {
           total+=block.size();
     }
    int rem=total%4;
    int k = (rem == 0) ? total/4 : total/4 + 1;
    cout<<"total : "<<total<<endl;
    int flag=-1;
    
    int re=1;
     for (const auto& block : memory) {
            
        for (uint8_t byte : block) {
        unsigned int address = start_address + ((k-1) * 4);
        if((re%4-1)==0){
        cout << hex << "0x" << setw(8) << setfill('0') << address <<"  ";
        }
            if(flag<0){
                if(rem==0){
                }else{

            for(int i=0;i<4-rem;i++){
                cout<<"X ";
            flag++;
            re++;
            }
                }
            }
            cout << static_cast<int>(byte) << " ";
            if(re%4==0){
                cout<<endl;
                k--;
            }
            re++;   
        }
    
    }
    cout << endl;
 
//div



    return 0;
}

/*
#include <iostream>
#include <iomanip>

int main() {
    unsigned int start_address = 0x10000000;
    int num_addresses = 10;
    int bytes_per_address = 4;

    for (int i = 0; i < num_addresses; ++i) {
        unsigned int address = start_address + (i * bytes_per_address);
        std::cout << std::hex << "0x" << std::setw(8) << std::setfill('0') << address << std::endl;
    }

    return 0;
}

*/