/*main.c*/

//
// Project 02: main program for execution of nuPython.
// 
// Solution by Prof. Joe Hummel
// Northwestern University
// CS 211
//

// to eliminate warnings about stdlib in Visual Studio
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>  // true, false
#include <string.h>   // strcspn
#include <assert.h>

#include "token.h"    // token defs
#include "scanner.h"  // scanner
#include "util.h"     // panic
#include "tokenqueue.h"  // program in token form
#include "parser.h"      // parser checks program syntax
#include "ast.h"         // builds AST representation
#include "ram.h"         // memory for program execution

static bool execute(struct AST_STMT* program, struct RAM* memory);

//
// all_zeros
//
// Returns true if the given string contains all 0 digits,
// false otherwise.
//
bool all_zeros(char* s)
{
  for (int i = 0; i < strlen(s); i++)
    if (s[i] != '0')
      return false;

  return true;
}

// binary plus function when adding one variable to a literal

static bool binary_plus_one_var(struct AST_UNARY_EXPR* lhs_expr, struct AST_UNARY_EXPR* rhs_expr, char* variable_name, struct RAM* memory) {
    char* other_var_name = lhs_expr->types.variable_name;

    char* literal_value = rhs_expr->types.literal_value;

    struct RAM_CELL* cell = ram_get_cell_by_id(memory, other_var_name);

    if (cell == NULL) {
      // no such variable:
      printf("ERROR: name '%s' is not defined\n", other_var_name);
      return false;
    }

    if (((rhs_expr->unary_expr_type == AST_UNARY_EXPR_INT_LITERAL || rhs_expr->unary_expr_type == AST_UNARY_EXPR_REAL_LITERAL) && cell->ram_cell_type == RAM_TYPE_STR) || (rhs_expr->unary_expr_type == AST_UNARY_EXPR_STR_LITERAL && (cell->ram_cell_type == RAM_TYPE_INT || cell->ram_cell_type == RAM_TYPE_REAL))) {
      printf("ERROR: unsupported operand type(s) for +\n");
      return false;
    }
    else if (rhs_expr->unary_expr_type == AST_UNARY_EXPR_INT_LITERAL && cell->ram_cell_type == RAM_TYPE_INT) 
  {

    /*
    if (cell->ram_cell_type != RAM_TYPE_INT)
      panic("expecting integer variable on RHS of assignment (execute_assignment)"); */
    int value = atoi(literal_value);

    value = cell->types.i + value;

    ram_write_int_by_id(memory, variable_name, value);
  }
    else if (rhs_expr->unary_expr_type == AST_UNARY_EXPR_REAL_LITERAL && cell->ram_cell_type == RAM_TYPE_INT) {

      double value = strtod(literal_value, NULL);
      value = cell->types.i + value; 
      ram_write_real_by_id(memory, variable_name, value);

      }
      else if (cell->ram_cell_type == RAM_TYPE_REAL) {

        double value = strtod(literal_value, NULL);
        value = cell->types.d + value; // error is here: need to be able to add multiple cell types (double and int)
        ram_write_real_by_id(memory, variable_name, value);

          }
    else if (rhs_expr->unary_expr_type == AST_UNARY_EXPR_STR_LITERAL) {
      int len1 = strlen(literal_value); // Find the length of str1
      int len2 = strlen(cell->types.s); // Find length of str2
      int total_len = len1 + len2; 
      
      char *result = (char *)malloc(len1 + len2 + 1); // +1 for the null-terminator


      strcpy (result, cell->types.s);
      strcat (result, literal_value);
      ram_write_str_by_id(memory, variable_name, result);
      free(result);
      }
  else
  {
    panic("assignment unary_expr_type not supported (execute_assignment)");
  }
  return true;
}

//binary plus function when adding two variables

static bool binary_plus_two_var(struct AST_UNARY_EXPR* lhs_expr, struct AST_UNARY_EXPR* rhs_expr, char* variable_name, struct RAM* memory) {
    char* first_var_name = lhs_expr->types.variable_name;

    char* second_var_name = rhs_expr->types.variable_name;

    struct RAM_CELL* cell1 = ram_get_cell_by_id(memory, first_var_name);
    struct RAM_CELL* cell2 = ram_get_cell_by_id(memory, second_var_name);
  
    if (cell1 == NULL) {
      // no such variable:
      printf("ERROR: name '%s' is not defined\n", first_var_name);
      return false;
    }
  
    if (cell2 == NULL) {
      // no such variable:
      printf("ERROR: name '%s' is not defined\n", second_var_name);
      return false;
    }

    if (((cell1->ram_cell_type == RAM_TYPE_INT || cell1->ram_cell_type == RAM_TYPE_REAL) && cell2->ram_cell_type == RAM_TYPE_STR) || ((cell2->ram_cell_type == RAM_TYPE_INT || cell2->ram_cell_type == RAM_TYPE_REAL) && cell1->ram_cell_type == RAM_TYPE_STR)) {
      printf("ERROR: unsupported operand type(s) for +\n");
      return false;
    }
    else if (cell2->ram_cell_type == RAM_TYPE_INT && cell1->ram_cell_type == RAM_TYPE_INT) 
  {
    int value = cell1->types.i + cell2->types.i;

    ram_write_int_by_id(memory, variable_name, value);
  }
    else if (cell2->ram_cell_type == RAM_TYPE_REAL && cell1->ram_cell_type == RAM_TYPE_INT) {

      double value = cell1->types.i + cell2->types.d; 
      ram_write_real_by_id(memory, variable_name, value);

      }
      else if (cell1->ram_cell_type == RAM_TYPE_REAL && cell2->ram_cell_type == RAM_TYPE_INT) {

      double value = cell2->types.i + cell1->types.d; 
      ram_write_real_by_id(memory, variable_name, value);

      }
      else if (cell2->ram_cell_type == RAM_TYPE_REAL && cell1->ram_cell_type == RAM_TYPE_REAL) {

        double value = cell2->types.d + cell1->types.d; 
        ram_write_real_by_id(memory, variable_name, value);

          }
    else if (cell2->ram_cell_type == RAM_TYPE_STR) {
      int len1 = strlen(cell1->types.s); // Find the length of str1
      int len2 = strlen(cell2->types.s); // Find length of str2
      int total_len = len1 + len2; 

      char *result = (char *)malloc(len1 + len2 + 1); // +1 for the null-terminator


      strcpy (result, cell1->types.s);
      strcat (result, cell2->types.s);
      ram_write_str_by_id(memory, variable_name, result);
      free(result);
      }
  else
  {
    panic("assignment unary_expr_type not supported (execute_assignment)");
  }
  return true;
}

//binary plus function-called by execute, uses two previously defined
//binary plus functions

static bool binary_plus(struct AST_UNARY_EXPR* lhs_expr, struct AST_UNARY_EXPR* rhs_expr, char* variable_name, struct RAM* memory) {
  if (lhs_expr->unary_expr_type == AST_UNARY_EXPR_VARIABLE_ACCESS) {
    if (rhs_expr->unary_expr_type != AST_UNARY_EXPR_VARIABLE_ACCESS) {
      bool result = binary_plus_one_var(lhs_expr, rhs_expr, variable_name, memory);
      if (result == false) {
        return false;
      }
    }
    else if (rhs_expr->unary_expr_type == AST_UNARY_EXPR_VARIABLE_ACCESS) {
      bool result = binary_plus_two_var(lhs_expr, rhs_expr, variable_name, memory);
      if (result == false) {
        
        return false;
      }
    }
  }
  else
  {
    panic("assignment unary_expr_type not supported (execute_assignment)");
  }
  return true;
  
}

//
// execute_assignment_function_call
//
// Executes assignment statements that contain a 
// function call, returning true if execution is
// sucessful and false if not.
//
// Example: y = input("Enter an int>")
//          y = int(y)
//
static bool execute_assignment_function_call(struct AST_STMT* stmt, struct RAM* memory)
{
  assert(stmt->stmt_type == AST_ASSIGNMENT);

  struct AST_STMT_ASSIGNMENT* assignment = stmt->types.assignment;
  char* variable_name = assignment->variable_name;

  assert(assignment->expr->expr_type == AST_FUNCTION_CALL_EXPR);

  char* function_name = assignment->expr->types.function_call->function_name;
  struct AST_EXPR* parameter = assignment->expr->types.function_call->param;

  //
  // functions have exactly one parameter that's a simple <element>
  // in the BNF, e.g. identifier or literal.
  //
  assert(parameter->expr_type == AST_UNARY_EXPR);

  struct AST_UNARY_EXPR* unary_expr = parameter->types.unary_expr;

  if (strcmp(function_name, "input") == 0)
  {
    //
    // x = input("...")
    //
    // 1. prompt the user:
    //
    if (unary_expr->unary_expr_type == AST_UNARY_EXPR_STR_LITERAL)
      printf("%s", unary_expr->types.literal_value);
    else
      panic("unsupported input() parameter unary expr type (execute_assignment_function_call)");

    //
    // 2. input from keyboard and assign to variable:
    //
    char line[256];
    fgets(line, sizeof(line), stdin);
    line[strcspn(line, "\r\n")] = '\0';  // delete EOL chars

    ram_write_str_by_id(memory, variable_name, line);
  }
  else if (strcmp(function_name, "int") == 0)
  {
    //
    // x = int(rhs)
    //
    if (unary_expr->unary_expr_type == AST_UNARY_EXPR_VARIABLE_ACCESS)
    {
      //
      // rhs => "right-hand-side", i.e. the variable on the rhs of =
      //
      char* rhs_var_name = parameter->types.unary_expr->types.variable_name;

      //
      // 1. retrieve rhs value
      //
      struct RAM_CELL* rhs_cell = ram_get_cell_by_id(memory, rhs_var_name);

      if (rhs_cell == NULL) {
        // no such variable:
        printf("ERROR: name '%s' is not defined\n", rhs_var_name);
        return false;
      }

      if (rhs_cell->ram_cell_type != RAM_TYPE_STR)
        panic("int() requires a string value (execute_assignment)");

      //
      // 2. value is a string, convert to int:
      //
      int value = atoi(rhs_cell->types.s);
      if (value == 0) {
        //
        // atoi() likely failed, but what if string is literally "0"?
        // check for this...
        //
        if (all_zeros(rhs_cell->types.s)) {
          // okay
        }
        else {
          printf("ERROR: invalid numeric string for int()\n");
          return false;
        }
      }

      // 
      // 3. write to memory
      //
      ram_write_int_by_id(memory, variable_name, value);
    }
    else {
      panic("unsupported int(unary_expr_type) (execute_assignment_function_call)");
    }
  }
  else {
    panic("function not supported within assignment (execute_assignment_function_call)");
  }

  //
  // done, success:
  //
  return true;
}

//
// execute_assignment
//
// Executes assignment statements, returning true 
// if execution is sucessful and false if not.
// 
// Example: x = 123
//          y = input("Enter an int>")
//          y = int(y)
//          x = x + 1
//          z = y + 12
//
static bool execute_assignment(struct AST_STMT* stmt, struct RAM* memory)
{
  assert(stmt->stmt_type == AST_ASSIGNMENT);

  struct AST_STMT_ASSIGNMENT* assignment = stmt->types.assignment;
  char* variable_name = assignment->variable_name;

  if (assignment->expr->expr_type == AST_UNARY_EXPR)
  {
    //
    // x = int_literal
    // 
    struct AST_UNARY_EXPR* unary_expr = assignment->expr->types.unary_expr;

    if (unary_expr->unary_expr_type == AST_UNARY_EXPR_INT_LITERAL)
    {
      int value = atoi(unary_expr->types.literal_value);

      ram_write_int_by_id(memory, variable_name, value);
    }
    else if (unary_expr->unary_expr_type == AST_UNARY_EXPR_STR_LITERAL) {
      ram_write_str_by_id(memory, variable_name, unary_expr->types.literal_value);
    }
    else if (unary_expr->unary_expr_type == AST_UNARY_EXPR_REAL_LITERAL) {
        double num = strtod(unary_expr->types.literal_value, NULL);
        
        ram_write_real_by_id(memory, variable_name, num);
      }
      else if (unary_expr->unary_expr_type == AST_UNARY_EXPR_VARIABLE_ACCESS) {
        char* var_name = unary_expr->types.variable_name;
        struct RAM_CELL* cell = ram_get_cell_by_id(memory, var_name);
    
        if (cell == NULL) {
          // no such variable:
          printf("ERROR: name '%s' is not defined\n", var_name);
          return false;
        }
        if (cell->ram_cell_type == RAM_TYPE_INT) {
        ram_write_int_by_id(memory, variable_name, cell->types.i);
        }
        else if (cell->ram_cell_type == RAM_TYPE_STR) {
          ram_write_str_by_id(memory, variable_name, cell->types.s);
        }
        else if (cell->ram_cell_type == RAM_TYPE_REAL) {
          ram_write_real_by_id(memory, variable_name, cell->types.d);
        }
      }
    else
    {
      panic("assignment unary_expr_type not supported (execute_assignment)");
    }
  }
  else if (assignment->expr->expr_type == AST_BINARY_EXPR)
  {
    //
    // variable_name = other_var_name + int_literal
    //
    struct AST_BINARY_EXPR* binary_expr = assignment->expr->types.binary_expr;

    struct AST_UNARY_EXPR* lhs_expr = binary_expr->lhs;
    struct AST_UNARY_EXPR* rhs_expr = binary_expr->rhs;

    if (binary_expr->op == AST_BINARY_EXPR_PLUS)
    {
      return binary_plus(lhs_expr, rhs_expr, variable_name, memory);
      
    }
    else {
      panic("unsupported binary expression (execute_assignment)");
    }
  }
  else if (assignment->expr->expr_type == AST_FUNCTION_CALL_EXPR)
  {
    bool success = execute_assignment_function_call(stmt, memory);

    if (!success)
      return false;
  }
  else {
    panic("assignment expr_type not supported (execute_assignment)");
  }
  //
  // done, success:
  //
  return true;
}


//
// execute_function_call
//
// Executes the stmt denoting a function call, such as print(...).
// Returns true if successful, false if not.
// 
// Example: print("a string")
//          print(123)
//          print(x)
//

static bool execute_function_call(struct AST_STMT* stmt, struct RAM* memory)
{
  assert(stmt->stmt_type == AST_FUNCTION_CALL);

  char* function_name = stmt->types.function_call->function_name;
  struct AST_EXPR* parameter = stmt->types.function_call->param;

  //
  // functions have exactly one parameter that's a simple <element>
  // in the BNF, e.g. identifier or literal.
  //
  assert(parameter->expr_type == AST_UNARY_EXPR);

  struct AST_UNARY_EXPR* unary_expr = parameter->types.unary_expr;

  if (strcmp(function_name, "print") == 0)
  {
    //
    // execute print(???)
    //
    if (unary_expr->unary_expr_type == AST_UNARY_EXPR_INT_LITERAL ||
      unary_expr->unary_expr_type == AST_UNARY_EXPR_REAL_LITERAL ||
      unary_expr->unary_expr_type == AST_UNARY_EXPR_STR_LITERAL)
    {
      //
      // print(literal)
      //
      printf("%s\n", unary_expr->types.literal_value);
    }
    else if (unary_expr->unary_expr_type == AST_UNARY_EXPR_VARIABLE_ACCESS)
    {
      //
      // print(variable)
      //
      char* identifier = unary_expr->types.variable_name;

      struct RAM_CELL* memory_cell = ram_get_cell_by_id(memory, identifier);
      if (memory_cell == NULL)
      {
        printf("ERROR: name '%s' is not defined\n", identifier);
        return false;
      }
      else if (memory_cell->ram_cell_type == RAM_TYPE_INT)
        printf("%d\n", memory_cell->types.i);
      else if (memory_cell->ram_cell_type == RAM_TYPE_STR)
        printf("%s\n", memory_cell->types.s);
      else if (memory_cell->ram_cell_type == RAM_TYPE_REAL)
        printf("%lf\n", memory_cell->types.d);
      else
        panic("unsupported variable type (execute_function_call)");
    }
    else {
      panic("unsupported print(unary_expr_type) (execute_function_call)");
    }
  }
  else {
    panic("unsupported function (execute_function_call)");
  }

  //
  // done, success:
  //
  return true;
}

static bool execute_while_loop(struct AST_STMT* stmt, struct RAM* memory) {
  assert(stmt->stmt_type == AST_WHILE_LOOP);
  struct AST_STMT_WHILE_LOOP* while_loop = stmt->types.while_loop;
  struct AST_STMT* body = while_loop->body;
  struct AST_EXPR* condition = while_loop->condition;
  struct AST_BINARY_EXPR* binary_expr = condition->types.binary_expr;
  struct AST_UNARY_EXPR* lhs_expr = binary_expr->lhs;
  struct AST_UNARY_EXPR* rhs_expr = binary_expr->rhs;
  char* var_name = lhs_expr->types.variable_name;

  char* literal_value = rhs_expr->types.literal_value;

  struct RAM_CELL* cell = ram_get_cell_by_id(memory, var_name);

  if (cell == NULL) {
    // no such variable:
    printf("ERROR: name '%s' is not defined\n", var_name);
    return false;
  }
  bool success = true;
  if (rhs_expr->unary_expr_type == AST_UNARY_EXPR_INT_LITERAL && cell->ram_cell_type == RAM_TYPE_INT) {
    int value = atoi(literal_value);
    while (value != cell->types.i && success == true) {
      success = execute(body, memory);
    }
  }
  else if (rhs_expr->unary_expr_type == AST_UNARY_EXPR_REAL_LITERAL && cell->ram_cell_type == RAM_TYPE_INT) {

    double value = strtod(literal_value, NULL);
    while (value != cell->types.i && success == true) {
      success = execute(body, memory);
    }

  }
  else if (cell->ram_cell_type == RAM_TYPE_REAL) {
    
    double value = strtod(literal_value, NULL);
    while (value != cell->types.d && success == true) {
      success = execute(body, memory);
    }
    

  }
  else if (rhs_expr->unary_expr_type == AST_UNARY_EXPR_STR_LITERAL && cell->ram_cell_type == RAM_TYPE_STR) {
    while (strcmp(literal_value, cell->types.s) != 0 && success == true) {
      success = execute(body, memory);
    }
  }
  else {
    printf("ERROR: unsupported operand type(s) for !=\n");
    return false;
  }
  if (success == false) {
      return false;

  }

  return true;
}

//
// execute
//
// Executes the given nuPython program.
//
static bool execute(struct AST_STMT* program, struct RAM* memory)
{
  //
  // loop through the program stmt by stmt:
  //
  struct AST_STMT* cur = program;

  while (cur != NULL)
  {
    bool success = false;

    // printf("stmt type %d\n", cur->stmt_type);

    if (cur->stmt_type == AST_ASSIGNMENT)
    {
      success = execute_assignment(cur, memory);
    }
    else if (cur->stmt_type == AST_DEREF_PTR_ASSIGNMENT)
    {
      panic("deref ptr assignment not yet supported (execute)");
    }
    else if (cur->stmt_type == AST_FUNCTION_CALL)
    {
      success = execute_function_call(cur, memory);
    }
    else if (cur->stmt_type == AST_IF_THEN_ELSE)
    {
      panic("if-then-else not yet supported (execute)");
    }
    else if (cur->stmt_type == AST_WHILE_LOOP)
    {
      success = execute_while_loop(cur, memory);
    }
    else if (cur->stmt_type == AST_PASS)
    {
      //
      // pass => do nothing!
      //
    }
    else
    {
      panic("unknown statement type?! (execute)");
    }

    //
    // did execution fail?
    //
    if (success == false) {
        return false;
    
    }  // yes, end execution:
      

    //
    // if execution was successful, advance to 
    // next stmt:
    //
    cur = cur->next;
  }

  //
  // done:
  //
  return true;
}

//
// main
//
int main(int argc, char* argv[])
{
  //
  // Ask the user for a filename, if they don't enter one
  // then we'll take input from the keyboard:
  //
  char filename[64];

  printf("Enter nuPython file (press ENTER to input from keyboard)>\n");

  fgets(filename, 64, stdin);  // safely read at most 64 chars
  filename[strcspn(filename, "\r\n")] = '\0';  // delete EOL chars e.g. \n

  FILE* input = NULL;
  bool  keyboardInput = false;

  if (strlen(filename) == 0) {
    //
    // input from the keyboard, aka stdin:
    //
    input = stdin;
    keyboardInput = true;
  }
  else {
    //
    // can we open the file?
    //
    input = fopen(filename, "r");

    if (input == NULL) // unable to open:
    {
      printf("**ERROR: unable to open input file '%s' for input.\n", filename);
      return 0;
    }

    keyboardInput = false;
  }

  //
  // input the tokens, either from keyboard or the given nuPython 
  // file; the "input" variable controls the source. the scanner will
  // stop and return EOS when the user enters $ or we reach EOF on
  // the nuPython file:
  //

  //
  // setup for parsing and execution:
  //
  if (keyboardInput)  // prompt the user if appropriate:
  {
    printf("nuPython input (enter $ when you're done)>\n");
  }

  parser_init();

  //
  // call parser to check program syntax:
  //
  struct TokenQueue* tokens = parser_parse(input);

  if (tokens == NULL)
  {
    // 
    // program has a syntax error, error msg already output:
    //
    printf("**parsing failed, cannot execute\n");
  }
  else
  {
    printf("**parsing successful\n");
    printf("**building AST...\n");

    struct AST_STMT* program = ast_build(tokens);
    
    printf("**starting execution...\n");

    //
    // if program is coming from the keyboard, consume
    // the rest of the input after the $ before we
    // start executing the python:
    //
    if (keyboardInput) {
      int c = fgetc(stdin);
      while (c != '\n')
        c = fgetc(stdin);
    }

    //
    // now execute the nuPython program:
    //
    struct RAM* memory = ram_init();

    execute(program, memory);
    
    
    ram_print(memory);
    ram_free(memory);
    ast_destroy(program);
    tokenqueue_destroy(tokens);
  }

  //
  // done:
  //
  if (!keyboardInput)
    fclose(input);
  
  
  
  
  return 0;
}
