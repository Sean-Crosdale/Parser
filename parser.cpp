/*
Sean Crosdale
10/3/2026
Project 1
Comp 360
*/
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>


using namespace std;

//Different types a token can fall under
 enum class TokenType{
    KEYWORD,
    IDENTIFIER,
    INT_LITERAL,
    FLOAT_LITERAL,
    OPERATOR,
    PUNCTUATOR,
    UNKNOWN,
    END_OF_FILE
 };

 struct Token{
    TokenType type;
    string value;
    int line,column;

    Token(TokenType t, const string& v, int ln, int col)
    :type(t),value(v),line(ln),column(col){
    }
 };

 string getTokenTypeName(TokenType token) {
   switch (token) {
      case TokenType::KEYWORD:
         return "KEYWORD";
      
      case TokenType::IDENTIFIER:
         return "IDENTIFIER";
      
      case TokenType::INT_LITERAL:
         return "INT_LITERAL";
      
      case TokenType::FLOAT_LITERAL:
         return "FLOAT_LITERAL";
      
      case TokenType::OPERATOR:
         return "OPERATOR";
      
      case TokenType::PUNCTUATOR:
      return "PUNCTUATOR";
      
      case TokenType::UNKNOWN:
         return "UNKNOWN";
      
      case TokenType::END_OF_FILE:
         return "EOF";
   }
   return "UNKNOWN";
}
class lexicalAnalyzer {
private:
   string input;
   size_t position;
   int line, column;

   unordered_set<string> keywords;

   void initKeywords(){
      keywords.insert("int");
      keywords.insert("float");
   }

   bool isWhitespace(char c){
      return c == ' ' || c == '\n' || c == '\t' || c == '\r';
   }

   bool isAlpha(char c){
      return c >= 'a' && c <= 'z' || (c >= 'A' && c <= 'Z');
   }

   bool isDigit(char c){
      return c >= '0' && c <= '9'; 
   }

   bool isAlphaNumeric(char c){
      return isDigit(c)|| isAlpha(c);
   }

   void advance() {
      if (position >= input.length()) {
         return;
      }

      if (input[position] == '\n') {
         line++;
         column = 1;
      }
      else {
         column++;
      }
      position++;
   }

   string readIdentifier() {
      size_t start = position;
      
      while (position < input.length() && isAlphaNumeric(input[position])) {
         advance();
      }
      
      return input.substr(start, position - start);
   }

   
   string readNumber(){
      size_t start = position;
      bool hasDecimal = false;

      while(position < input.length()){
         if(isDigit(input[position])){
            advance();
         }
         else if(input[position] == '.' && !hasDecimal){
            hasDecimal = true;
            advance();
         }
         else{
            break;
         }
      }
      return input.substr(start, position - start);
   }

public:
   lexicalAnalyzer(const string& source)
   :input(source), position(0), line(1), column(1)
   {
      initKeywords();
   }

   vector<Token> tokenize(){
      vector<Token> tokens;

      while(position < input.length()){

         char currChar = input[position];

         if(isWhitespace(currChar)){
            advance();
            continue;
         }

         int tokenLine = line;
         int tokenColumn = column;

         if(isAlpha(currChar)){
            
            string word = readIdentifier();

            if(keywords.find(word) != keywords.end()){
               tokens.emplace_back(TokenType::KEYWORD, word, tokenLine, tokenColumn);
            }
            else{
               tokens.emplace_back(TokenType::IDENTIFIER, word, tokenLine, tokenColumn);
            }

            continue;
         }

         if(isDigit(currChar)){
            
            string number = readNumber();

            if(number.find('.') != string::npos){
               tokens.emplace_back(TokenType :: FLOAT_LITERAL, number, tokenLine, tokenColumn);
            }
            else{
               tokens.emplace_back(TokenType :: INT_LITERAL, number, tokenLine, tokenColumn);
            }

            continue;
         }

         if(currChar == '+' || currChar == '-' || currChar == '*' || currChar =='/' || currChar == '='){
            tokens.emplace_back(TokenType:: OPERATOR, string(1,currChar), tokenLine, tokenColumn);
            advance();
            continue;
         }

         if(currChar == '(' || currChar == ')' || currChar == '{' || currChar =='}' || currChar == ';' || currChar == ','){
            tokens.emplace_back(TokenType ::PUNCTUATOR, string(1, currChar), tokenLine, tokenColumn);
            advance();
            continue;
         }

         tokens.emplace_back(TokenType::UNKNOWN, string(1, currChar), tokenLine, tokenColumn);
         advance();
      }

      tokens.emplace_back(TokenType::END_OF_FILE, "", line, column);
      return tokens;
   }
};



class Parser {
private:
    vector<Token> tokens;
    size_t current;
    bool errFound;

    Token& currToken() {
        return tokens[current];
    }

    void synError(const string& message) {
        if (!errFound) {
            cerr << "Syntax Error: " << message
                 << " near '" << currToken().value
                 << "' at line " << currToken().line
                 << ", column " << currToken().column << endl;
        }

        errFound = true;
    }

    bool match(const string& val) {
        if (currToken().value == val) {
            current++;
            return true;
        }

        return false;
    }

    bool isType() {
        return currToken().value == "float" ||
               currToken().value == "int";
    }

    // Function -> Type Identifier '(' Parameters ')' '{' Statements '}'

    bool Function() {

        // return type
        if (!isType()) {
            synError("Expected return type");
            return false;
        }

        current++;

        // function name
        if (currToken().type != TokenType::IDENTIFIER) {
            synError("Expected function name");
            return false;
        }

        current++;

        // (
        if (!match("(")) {
            synError("Expected '(' after function name");
            return false;
        }

        // parameters
        if (!Parameters()) {
            return false;
        }

        // )
        if (!match(")")) {
            synError("Expected ')'");
            return false;
        }

        // {
        if (!match("{")) {
            synError("Expected '{'");
            return false;
        }

        // statements
        if (!Statements()) {
            return false;
        }

        // }
        if (!match("}")) {
            synError("Expected '}'");
            return false;
        }

        return true;
    }

    // Parameters -> Type Identifier { ',' Type Identifier }

    bool Parameters() {

        // Empty parameter list
        if (currToken().value == ")") {
            return true;
        }

        if (!isType()) {
            synError("Expected parameter type");
            return false;
        }

        current++;

        if (currToken().type != TokenType::IDENTIFIER) {
            synError("Expected parameter name");
            return false;
        }

        current++;

        while (match(",")) {

            if (!isType()) {
                synError("Expected parameter type after ','");
                return false;
            }

            current++;

            if (currToken().type != TokenType::IDENTIFIER) {
                synError("Expected parameter name");
                return false;
            }

            current++;
        }

        return true;
    }

    // Statements -> { Declaration | Assignment }

    bool Statements() {

        while (currToken().value != "}") {

            if (currToken().type == TokenType::END_OF_FILE) {
                synError("Expected '}' before end of file");
                return false;
            }

            // Variable declaration
            if (isType()) {
                if (!Declaration()) {
                    return false;
                }
            }

            // Assignment
            else if (currToken().type == TokenType::IDENTIFIER) {
                if (!Assignment()) {
                    return false;
                }
            }

            else {
                synError("Expected declaration or assignment");
                return false;
            }
        }

        return true;
    }

    // Declaration -> Type Identifier ';'

    bool Declaration() {

        if (!isType()) {
            synError("Expected type");
            return false;
        }

        current++;

        if (currToken().type != TokenType::IDENTIFIER) {
            synError("Expected variable name");
            return false;
        }

        current++;

        if (!match(";")) {
            synError("Expected ';' after declaration");
            return false;
        }

        return true;
    }

    
    // Assignment -> Identifier '=' Expression ';'
    

    bool Assignment() {

        if (currToken().type != TokenType::IDENTIFIER) {
            synError("Expected identifier");
            return false;
        }

        current++;

        if (!match("=")) {
            synError("Expected '='");
            return false;
        }

        if (!Expression()) {
            return false;
        }

        if (!match(";")) {
            synError("Expected ';' after assignment");
            return false;
        }

        return true;
    }

    
    // Expression -> Term { (+ | -) Term }
    

    bool Expression() {

        if (!Term()) {
            return false;
        }

        while (currToken().value == "+" ||
               currToken().value == "-") {

            current++;

            if (!Term()) {
                synError("Expected expression after '+' or '-'");
                return false;
            }
        }

        return true;
    }

    
   
    // Term -> Factor { (* | /) Factor }
    

    bool Term() {

        if (!Factor()) {
            return false;
        }

        while (currToken().value == "*" ||
               currToken().value == "/") {

            current++;

            if (!Factor()) {
                synError("Expected expression after '*' or '/'");
                return false;
            }
        }

        return true;
    }

    // factor -> id, int, float or '('expr ')'

    bool Factor() {

        if (currToken().type == TokenType::IDENTIFIER) {
            current++;
            return true;
        }

        if (currToken().type == TokenType::INT_LITERAL) {
            current++;
            return true;
        }

        if (currToken().type == TokenType::FLOAT_LITERAL) {
            current++;
            return true;
        }

        if (match("(")) {

            if (!Expression()) {
                return false;
            }

            if (!match(")")) {
                synError("Expected ')'");
                return false;
            }

            return true;
        }

        synError("Expected identifier, number, or '('");
        return false;
    }

public:

    Parser(const vector<Token>& tokenList)
        : tokens(tokenList), current(0), errFound(false) {}

    bool parse() {

        if (tokens.empty()) {
            return false;
        }

        if (!Function()) {
            return false;
        }

        if (currToken().type != TokenType::END_OF_FILE) {
            synError("Unexpected token after function");
            return false;
        }

        return !errFound;
    }
};

int main(int argc, char* argv[]){
   if(argc != 2){
      return 1;
   }

   ifstream inputFile(argv[1]);
   
   if(!inputFile){
      return 1;
   }

   stringstream buffer;
   buffer << inputFile.rdbuf();
   string sourceCode = buffer.str();
   inputFile.close();

   if(sourceCode.empty()){
      return 1;
   }

   lexicalAnalyzer lexer(sourceCode);

   vector<Token> tokens = lexer.tokenize();

   bool lexError = false;
   
   for(const Token &token: tokens){

      if(token.type == TokenType::END_OF_FILE){
         continue;
      }

      cout << getTokenTypeName(token.type) << " : " << token.value << endl;

      if(token.type == TokenType::UNKNOWN){
         cerr << "Lexical Error: unkown token " << token.value << " at line" << token.line << ", column " <<token.column << endl;
         lexError = true;
      }
   }
   //Parser won't run if invalid characters are found
   if(lexError){
      cout << "Grammar wasn't derived from lexical analyzer" << endl;
      return 1;
   }

   Parser parser(tokens);

   bool success = parser.parse();
   
   if(success){
      cout <<"\nParsing successful" << endl;
      cout <<"Grammar derived from lexical analyzer" << endl;
      return 0;
   }

   else{
      cout <<"\nParsing failed" << endl;
      cout <<"Grammar wasn't derived from lexical analyzer" << endl;
      return 1;
   }
}
