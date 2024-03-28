#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <cstdlib>    // For std::exit
#include <cstdbool>   // For true, false in C++ standard library
#include <cstring>    // For std::strlen
#include <cassert>

#include "token.hpp"       // Token definitions
#include "scanner.hpp"     // Scanner
#include "util.hpp"        // panic
#include "tokenqueue.hpp"  // Program in token form
#include "parser.hpp"      // Parser checks program syntax
#include "ast.hpp"         // Builds AST representation
#include "ram.hpp"         // Memory for program execution

static bool execute(AST_STMT* program, RAM* memory);

//
// all_zeros
//
// Returns true if the given string contains all 0 digits,
// false otherwise.
//
bool all_zeros(char* s) {
  for (int i = 0; i < std::strlen(s); i++)
    if (s[i] != '0')
      return false;

  return true;
}

// binary plus function when adding one variable to a literal

static bool binary_plus_one_var(AST_UNARY_EXPR* lhs_expr, AST_UNARY_EXPR* rhs_expr, char* variable_name, RAM* memory) {
    char* other_var_name = lhs_expr->types.variable_name;
    char* literal_value = rhs_expr->types.literal_value;
    RAM_CELL* cell = ram_get_cell_by_id(memory, other_var_name);

    if (cell == nullptr) {
      std::cerr << "ERROR: name '" << other_var_name << "' is not defined\n";
      return false;
    }

    if (((rhs_expr->unary_expr_type == AST_UNARY_EXPR_INT_LITERAL || rhs_expr->unary_expr_type == AST_UNARY_EXPR_REAL_LITERAL) && cell->ram_cell_type == RAM_TYPE_STR) || 
        (rhs_expr->unary_expr_type == AST_UNARY_EXPR_STR_LITERAL && (cell->ram_cell_type == RAM_TYPE_INT || cell->ram_cell_type == RAM_TYPE_REAL))) {
      std::cerr << "ERROR: unsupported operand type(s) for +\n";
      return false;
    }
    else if (rhs_expr->unary_expr_type == AST_UNARY_EXPR_INT_LITERAL && cell->ram_cell_type == RAM_TYPE_INT) {
        int value = std::atoi(literal_value);
        value = cell->types.i + value;
        ram_write_int_by_id(memory, variable_name, value);
    }
    else if (rhs_expr->unary_expr_type == AST_UNARY_EXPR_REAL_LITERAL && cell->ram_cell_type == RAM_TYPE_INT) {
        double value = std::strtod(literal_value, nullptr);
        value = cell->types.i + value;
        ram_write_real_by_id(memory, variable_name, value);
    }
    else if (cell->ram_cell_type == RAM_TYPE_REAL) {
        double value = std::strtod(literal_value, nullptr);
        value = cell->types.d + value;
        ram_write_real_by_id(memory, variable_name, value);
    }
    else if (rhs_expr->unary_expr_type == AST_UNARY_EXPR_STR_LITERAL) {
        std::string result = cell->types.s;
        result += literal_value;
        ram_write_str_by_id(memory, variable_name, result.c_str());
    }
    else {
        panic("assignment unary_expr_type not supported (execute_assignment)");
    }
    return true;
}

//binary plus function when adding two variables

static bool binary_plus_two_var(AST_UNARY_EXPR* lhs_expr, AST_UNARY_EXPR* rhs_expr, char* variable_name, RAM* memory) {
    std::string first_var_name = lhs_expr->types.variable_name;
    std::string second_var_name = rhs_expr->types.variable_name;

    RAM_CELL* cell1 = ram_get_cell_by_id(memory, first_var_name.c_str());
    RAM_CELL* cell2 = ram_get_cell_by_id(memory, second_var_name.c_str());

    if (cell1 == nullptr) {
        std::cerr << "ERROR: name '" << first_var_name << "' is not defined\n";
        return false;
    }

    if (cell2 == nullptr) {
        std::cerr << "ERROR: name '" << second_var_name << "' is not defined\n";
        return false;
    }

    if (((cell1->ram_cell_type == RAM_TYPE_INT || cell1->ram_cell_type == RAM_TYPE_REAL) && cell2->ram_cell_type == RAM_TYPE_STR) ||
        ((cell2->ram_cell_type == RAM_TYPE_INT || cell2->ram_cell_type == RAM_TYPE_REAL) && cell1->ram_cell_type == RAM_TYPE_STR)) {
        std::cerr << "ERROR: unsupported operand type(s) for +\n";
        return false;
    }
    else if (cell2->ram_cell_type == RAM_TYPE_INT && cell1->ram_cell_type == RAM_TYPE_INT) {
        int value = cell1->types.i + cell2->types.i;
        ram_write_int_by_id(memory, variable_name, value);
    }
    else if ((cell2->ram_cell_type == RAM_TYPE_REAL && cell1->ram_cell_type == RAM_TYPE_INT) || 
             (cell1->ram_cell_type == RAM_TYPE_REAL && cell2->ram_cell_type == RAM_TYPE_INT)) {
        double value = static_cast<double>(cell1->types.i) + cell2->types.d;
        ram_write_real_by_id(memory, variable_name, value);
    }
    else if (cell2->ram_cell_type == RAM_TYPE_REAL && cell1->ram_cell_type == RAM_TYPE_REAL) {
        double value = cell2->types.d + cell1->types.d;
        ram_write_real_by_id(memory, variable_name, value);
    }
    else if (cell2->ram_cell_type == RAM_TYPE_STR) {
        std::string result = cell1->types.s + cell2->types.s; // Concatenate strings
        ram_write_str_by_id(memory, variable_name, result.c_str());
    }
    else {
        panic("assignment unary_expr_type not supported (execute_assignment)");
    }
    return true;
}

//binary plus function-called by execute, uses two previously defined
//binary plus functions

static bool binary_plus(AST_UNARY_EXPR* lhs_expr, AST_UNARY_EXPR* rhs_expr, const std::string& variable_name, RAM* memory) {
  if (lhs_expr->unary_expr_type == AST_UNARY_EXPR_VARIABLE_ACCESS) {
    if (rhs_expr->unary_expr_type != AST_UNARY_EXPR_VARIABLE_ACCESS) {
      bool result = binary_plus_one_var(lhs_expr, rhs_expr, variable_name, memory);
      if (!result) {
        return false;
      }
    } else if (rhs_expr->unary_expr_type == AST_UNARY_EXPR_VARIABLE_ACCESS) {
      bool result = binary_plus_two_var(lhs_expr, rhs_expr, variable_name, memory);
      if (!result) {
        return false;
      }
    }
  } else {
    panic("Assignment unary_expr_type not supported (execute_assignment)");
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

static bool execute_assignment_function_call(AST_STMT* stmt, RAM* memory) {
    assert(stmt->stmt_type == AST_ASSIGNMENT);

    AST_STMT_ASSIGNMENT* assignment = stmt->types.assignment;
    std::string variable_name = assignment->variable_name;

    assert(assignment->expr->expr_type == AST_FUNCTION_CALL_EXPR);

    std::string function_name = assignment->expr->types.function_call->function_name;
    AST_EXPR* parameter = assignment->expr->types.function_call->param;

    // Asserting parameter is a unary expression
    assert(parameter->expr_type == AST_UNARY_EXPR);

    AST_UNARY_EXPR* unary_expr = parameter->types.unary_expr;

    if (function_name == "input") {
        // x = input("...")
        if (unary_expr->unary_expr_type == AST_UNARY_EXPR_STR_LITERAL)
            std::cout << unary_expr->types.literal_value;
        else
            panic("Unsupported input() parameter unary expr type (execute_assignment_function_call)");

        // Input from keyboard and assign to variable
        std::string line;
        std::getline(std::cin, line);
        ram_write_str_by_id(memory, variable_name, line);
    }
    else if (function_name == "int") {
        // x = int(rhs)
        if (unary_expr->unary_expr_type == AST_UNARY_EXPR_VARIABLE_ACCESS) {
            std::string rhs_var_name = parameter->types.unary_expr->types.variable_name;

            // Retrieve rhs value
            RAM_CELL* rhs_cell = ram_get_cell_by_id(memory, rhs_var_name);

            if (rhs_cell == nullptr) {
                std::cerr << "ERROR: name '" << rhs_var_name << "' is not defined\n";
                return false;
            }

            if (rhs_cell->ram_cell_type != RAM_TYPE_STR)
                panic("int() requires a string value (execute_assignment)");

            // Convert string to int
            int value = std::stoi(rhs_cell->types.s); // Using std::stoi for conversion and error handling
            // Additional error handling could be implemented here if std::stoi's exceptions are not sufficient

            // Write to memory
            ram_write_int_by_id(memory, variable_name, value);
        }
        else {
            panic("Unsupported int(unary_expr_type) (execute_assignment_function_call)");
        }
    }
    else {
        panic("Function not supported within assignment (execute_assignment_function_call)");
    }

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

static bool execute_assignment(AST_STMT* stmt, RAM* memory) {
    assert(stmt->stmt_type == AST_ASSIGNMENT);

    AST_STMT_ASSIGNMENT* assignment = stmt->types.assignment;
    std::string variable_name = assignment->variable_name;

    if (assignment->expr->expr_type == AST_UNARY_EXPR) {
        AST_UNARY_EXPR* unary_expr = assignment->expr->types.unary_expr;

        if (unary_expr->unary_expr_type == AST_UNARY_EXPR_INT_LITERAL) {
            int value = std::atoi(unary_expr->types.literal_value.c_str());
            ram_write_int_by_id(memory, variable_name, value);
        } else if (unary_expr->unary_expr_type == AST_UNARY_EXPR_STR_LITERAL) {
            ram_write_str_by_id(memory, variable_name, unary_expr->types.literal_value);
        } else if (unary_expr->unary_expr_type == AST_UNARY_EXPR_REAL_LITERAL) {
            double num = std::strtod(unary_expr->types.literal_value.c_str(), nullptr);
            ram_write_real_by_id(memory, variable_name, num);
        } else if (unary_expr->unary_expr_type == AST_UNARY_EXPR_VARIABLE_ACCESS) {
            std::string var_name = unary_expr->types.variable_name;
            RAM_CELL* cell = ram_get_cell_by_id(memory, var_name);

            if (cell == nullptr) {
                std::cerr << "ERROR: name '" << var_name << "' is not defined\n";
                return false;
            }
            if (cell->ram_cell_type == RAM_TYPE_INT) {
                ram_write_int_by_id(memory, variable_name, cell->types.i);
            } else if (cell->ram_cell_type == RAM_TYPE_STR) {
                ram_write_str_by_id(memory, variable_name, cell->types.s);
            } else if (cell->ram_cell_type == RAM_TYPE_REAL) {
                ram_write_real_by_id(memory, variable_name, cell->types.d);
            }
        } else {
            panic("Assignment unary_expr_type not supported (execute_assignment)");
        }
    } else if (assignment->expr->expr_type == AST_BINARY_EXPR) {
        AST_BINARY_EXPR* binary_expr = assignment->expr->types.binary_expr;
        AST_UNARY_EXPR* lhs_expr = binary_expr->lhs;
        AST_UNARY_EXPR* rhs_expr = binary_expr->rhs;

        if (binary_expr->op == AST_BINARY_EXPR_PLUS) {
            return binary_plus(lhs_expr, rhs_expr, variable_name, memory);
        } else {
            panic("Unsupported binary expression (execute_assignment)");
        }
    } else if (assignment->expr->expr_type == AST_FUNCTION_CALL_EXPR) {
        return execute_assignment_function_call(stmt, memory);
    } else {
        panic("Assignment expr_type not supported (execute_assignment)");
    }

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


static bool execute_function_call(AST_STMT* stmt, RAM* memory) {
    assert(stmt->stmt_type == AST_FUNCTION_CALL);

    std::string function_name = stmt->types.function_call->function_name;
    AST_EXPR* parameter = stmt->types.function_call->param;

    // Functions have exactly one parameter that's a simple <element> in the BNF
    assert(parameter->expr_type == AST_UNARY_EXPR);

    AST_UNARY_EXPR* unary_expr = parameter->types.unary_expr;

    if (function_name == "print") {
        if (unary_expr->unary_expr_type == AST_UNARY_EXPR_INT_LITERAL ||
            unary_expr->unary_expr_type == AST_UNARY_EXPR_REAL_LITERAL ||
            unary_expr->unary_expr_type == AST_UNARY_EXPR_STR_LITERAL) {
            // Execute print(literal)
            std::cout << unary_expr->types.literal_value << std::endl;
        } else if (unary_expr->unary_expr_type == AST_UNARY_EXPR_VARIABLE_ACCESS) {
            // Execute print(variable)
            std::string identifier = unary_expr->types.variable_name;
            RAM_CELL* memory_cell = ram_get_cell_by_id(memory, identifier);

            if (memory_cell == nullptr) {
                std::cout << "ERROR: name '" << identifier << "' is not defined\n";
                return false;
            } else if (memory_cell->ram_cell_type == RAM_TYPE_INT) {
                std::cout << memory_cell->types.i << std::endl;
            } else if (memory_cell->ram_cell_type == RAM_TYPE_STR) {
                std::cout << memory_cell->types.s << std::endl;
            } else if (memory_cell->ram_cell_type == RAM_TYPE_REAL) {
                std::cout << memory_cell->types.d << std::endl;
            } else {
                panic("Unsupported variable type (execute_function_call)");
            }
        } else {
            panic("Unsupported print(unary_expr_type) (execute_function_call)");
        }
    } else {
        panic("Unsupported function (execute_function_call)");
    }

    return true;
}

// execute_while_loop
//
// Executes the given while loop
//

static bool execute_while_loop(AST_STMT* stmt, RAM* memory) {
    assert(stmt->stmt_type == AST_WHILE_LOOP);
    AST_STMT_WHILE_LOOP* while_loop = stmt->types.while_loop;
    AST_STMT* body = while_loop->body;
    AST_EXPR* condition = while_loop->condition;
    AST_BINARY_EXPR* binary_expr = condition->types.binary_expr;
    AST_UNARY_EXPR* lhs_expr = binary_expr->lhs;
    AST_UNARY_EXPR* rhs_expr = binary_expr->rhs;
    std::string var_name = lhs_expr->types.variable_name;

    std::string literal_value = rhs_expr->types.literal_value;

    RAM_CELL* cell = ram_get_cell_by_id(memory, var_name);

    if (cell == nullptr) {
        // No such variable
        std::cerr << "ERROR: name '" << var_name << "' is not defined\n";
        return false;
    }
    bool success = true;
    if (rhs_expr->unary_expr_type == AST_UNARY_EXPR_INT_LITERAL && cell->ram_cell_type == RAM_TYPE_INT) {
        int value = std::atoi(literal_value.c_str());
        while (value != cell->types.i && success) {
            success = execute(body, memory); // Assuming execute is adapted for C++
        }
    }
    else if (rhs_expr->unary_expr_type == AST_UNARY_EXPR_REAL_LITERAL && cell->ram_cell_type == RAM_TYPE_INT) {
        double value = std::strtod(literal_value.c_str(), nullptr);
        while (value != cell->types.i && success) {
            success = execute(body, memory);
        }
    }
    else if (cell->ram_cell_type == RAM_TYPE_REAL) {
        double value = std::strtod(literal_value.c_str(), nullptr);
        while (value != cell->types.d && success) {
            success = execute(body, memory);
        }
    }
    else if (rhs_expr->unary_expr_type == AST_UNARY_EXPR_STR_LITERAL && cell->ram_cell_type == RAM_TYPE_STR) {
        while (literal_value != cell->types.s && success) {
            success = execute(body, memory);
        }
    }
    else {
        std::cerr << "ERROR: unsupported operand type(s) for !=\n";
        return false;
    }
    if (!success) {
        return false;
    }

    return true;
}

//
// execute
//
// Executes the given program.
//

static bool execute(AST_STMT* program, RAM* memory) {
    AST_STMT* cur = program;

    while (cur != nullptr) {
        bool success = true;

        // std::cout << "stmt type " << cur->stmt_type << std::endl;

        switch (cur->stmt_type) {
            case AST_ASSIGNMENT:
                success = execute_assignment(cur, memory);
                break;
            case AST_DEREF_PTR_ASSIGNMENT:
                panic("deref ptr assignment not yet supported (execute)");
                break;
            case AST_FUNCTION_CALL:
                success = execute_function_call(cur, memory);
                break;
            case AST_IF_THEN_ELSE:
                panic("if-then-else not yet supported (execute)");
                break;
            case AST_WHILE_LOOP:
                success = execute_while_loop(cur, memory);
                break;
            case AST_PASS:
                // pass => do nothing!
                break;
            default:
                panic("unknown statement type?! (execute)");
                break;
        }

        if (!success) {
            // If execution failed, return false.
            return false;
        }

        // If execution was successful, advance to the next statement.
        cur = cur->next;
    }

    // Done:
    return true;
}

//
// main
//

int main(int argc, char* argv[]) {
    std::string filename;

    std::cout << "Enter nuPython file (press ENTER to input from keyboard)>\n";
    std::getline(std::cin, filename); // Read the filename or input command

    std::istream* input = &std::cin; // Default to standard input
    std::ifstream file;

    if (!filename.empty()) {
        file.open(filename);
        if (!file) {
            std::cerr << "**ERROR: unable to open input file '" << filename << "' for input.\n";
            return 0;
        }
        input = &file; // Use file input if a filename is provided
    }

    if (input == &std::cin) {
        std::cout << "nuPython input (enter $ when you're done)>\n";
    }

    parser_init();

    TokenQueue* tokens = parser_parse(*input); // Assuming parser_parse now takes std::istream&

    if (tokens == nullptr) {
        std::cout << "**parsing failed, cannot execute\n";
    } else {
        std::cout << "**parsing successful\n";
        std::cout << "**building AST...\n";

        AST_STMT* program = ast_build(tokens);

        std::cout << "**starting execution...\n";

        if (input == &std::cin) {
            // Consume the rest of the input after $ if input is from keyboard
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        RAM* memory = ram_init();

        execute(program, memory);

        ram_print(memory);
        ram_free(memory);
        ast_destroy(program);
        tokenqueue_destroy(tokens);
    }

    // File is automatically closed when going out of scope

    return 0;
}
