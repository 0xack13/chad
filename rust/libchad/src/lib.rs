#![allow(non_camel_case_types)]
#![allow(dead_code)]
#![allow(unused_imports)]

use std::ffi::{CStr, CString};
use std::io::Write;

use libc::{c_char, c_float, c_int, c_long};

use std::time::{SystemTime, UNIX_EPOCH};

use std::io::{self, BufRead, BufReader};
use std::process::{Command, Stdio};

extern "C" {
    fn log_print();

    // fn scope_get_variable_definition(
    //     tracker: *mut tracker_t,
    //     scope: *mut scope_T,
    //     name: *mut CStr,
    // ) -> *mut AST_T;

    // fn visit(visitor: *mut visitor_T, node: *mut AST_T) -> *mut AST_T;
}

#[repr(C)]
pub enum operator_t {
    O_NONE,
    O_ADD,
    O_SUB,
    O_DIV,
    O_MUL,
}

#[repr(C)]
pub enum block_type_t {
    BLOCK_NONE,
    BLOCK_IF,
    BLOCK_FOR,
    BLOCK_LOOP,
    BLOCK_FN,
}

#[repr(C)]
pub enum conditional_operator_t {
    CON_NONE,
    CON_GT,
    CON_LT,
    CON_EQ,
    CON_NEQ,
    CON_GTE,
    CON_LTE,
}

#[repr(C)]
pub enum logical_operator_t {
    LO_NONE,
    LO_NOT,
    LO_AND,
    LO_OR,
}

#[repr(C)]
#[derive(Debug)]
pub enum ast_type_t {
    AST_VARIABLE_DEFINITION,
    AST_VARIABLE_REDEFINITION,
    AST_FUNCTION_DEFINITION,
    AST_RETURN_STATEMENT,
    AST_BREAK_STATEMENT,
    AST_USE_STATEMENT,
    AST_CONTINUE_STATEMENT,
    AST_VARIABLE,
    AST_FUNCTION_CALL,
    AST_CHAR,
    AST_STRING,
    AST_INT,
    AST_FLOAT,
    AST_BOOL,
    AST_NUMBER_RANGE,
    AST_ARITHMETIC_OPERATOR,
    AST_CONDITIONAL_OPERATOR,
    AST_EXPRESSION,
    AST_CONDITIONAL_EXPRESSION,
    AST_LOGICAL_OPERATOR,
    AST_BLOCK,
    AST_IF_STATEMENT,
    AST_FOR_LOOP,
    AST_LOOP_LOOP,
    AST_EOF,
    AST_NULL,
}

#[repr(C)]
pub enum tracker_mode_t {
    LEXER,
    PARSER,
    RUNTIME,
    MODE_NONE,
}

#[repr(C)]
pub struct tracker_t {
    pub line: libc::c_int,
    pub column: libc::c_int,
    pub filename: *mut libc::c_char,
    pub mode: tracker_mode_t,
}

#[repr(C)]
pub struct scope_t {
    function_definitions: Vec<*mut AST_T>,
    function_definitions_size: c_int,

    variable_definitions: Vec<*mut AST_T>,
    variable_definitions_size: c_int,

    return_type: ast_type_t,
    returned_value: *mut AST_T,

    loop_break: c_int,
    loop_continue: c_int,

    returned: c_int,

    parent_scope: *mut scope_t,
    tracker: *mut tracker_t,
}

#[repr(C)]
pub struct visitor_t {
    tracker: *mut tracker_t,
    returned: c_int,
    loop_break: c_int,
    loop_continue: c_int,
}

#[repr(C)]
pub struct AST_T {
    // META
    line: c_int,
    column: c_int,

    // TYPE
    r#type: ast_type_t,

    // SCOPE
    scope: *mut scope_t,

    // PARENT
    parent_block: *mut AST_T,

    // VARIABLE DEFINITION
    variable_definition_variable_name: *mut c_char,
    variable_definition_value: *mut AST_T,
    variable_definition_type: ast_type_t,

    // VARIABLE REDEFINITION
    variable_redefinition_variable_name: *mut c_char,
    variable_redefinition_value: *mut AST_T,
    variable_redefinition_type: ast_type_t,

    // FUNCTION_DEFINITION
    function_definition_body: *mut AST_T,
    function_definition_name: *mut c_char,
    function_definition_args: *mut *mut AST_T,
    function_definition_args_size: c_long,
    function_definition_is_variadic: c_int,

    // VARIABLE
    variable_name: *mut c_char,

    // FUNCTION_CALL
    function_call_name: *mut c_char,
    function_call_arguments: *mut *mut AST_T,
    function_call_arguments_size: c_long,

    // ACTUAL VALUES
    string_value: *mut c_char,
    char_value: c_char,
    int_value: c_int,
    bool_value: bool,
    float_value: c_float,

    // NUMBER RANGE
    number_range_start: *mut AST_T,
    number_range_end: *mut AST_T,

    // ARITHMETIC OPERATOR
    operator_value: operator_t,

    // CONDITIONAL OPERATOR
    conditional_operator_value: conditional_operator_t,

    // LOGICAL OPERATOR
    logical_operator_value: logical_operator_t,

    // BLOCK
    block_values: *mut *mut AST_T,
    block_size: c_long,
    current_block_item: c_int,
    block_type: block_type_t,
    main_block: c_int,

    // EXPRESSION
    expression_values: *mut *mut AST_T,
    expression_size: c_long,

    // CONDITIONAL EXPRESSION
    conditional_expression_values: *mut *mut AST_T,
    conditional_expression_size: c_long,

    // IF STATEMENT
    if_statement_condition: *mut AST_T,
    if_statement_body: *mut AST_T,
    has_else: c_int,
    else_body: *mut AST_T,
    else_if_count: c_long,
    else_if_values: *mut *mut AST_T,

    // FOR LOOP
    for_loop_counter: *mut AST_T,
    for_loop_body: *mut AST_T,
    for_loop_range: *mut AST_T,

    // LOOP LOOP
    loop_loop_body: *mut AST_T,

    // RETURN STATEMENT
    returned_value: *mut AST_T,

    // USE STATEMENT
    use_file_path: *mut c_char,
    use_function_name: *mut c_char,
}

#[no_mangle]
pub extern "C" fn lib_print(visited_ast: &mut AST_T) {
    match visited_ast.r#type {
        ast_type_t::AST_STRING => {
            let rust_str = unsafe { CStr::from_ptr(visited_ast.string_value) }
                .to_string_lossy()
                .to_owned();

            print!("{}", rust_str);
        }
        ast_type_t::AST_CHAR => {
            print!("{}", (visited_ast.char_value as u8) as char);
        }
        ast_type_t::AST_INT => {
            print!("{}", visited_ast.int_value);
        }
        ast_type_t::AST_FLOAT => {
            print!("{}", visited_ast.float_value);
        }
        ast_type_t::AST_BOOL => {
            if visited_ast.bool_value {
                print!("{}", true);
            } else {
                print!("{}", false);
            }
        }
        ast_type_t::AST_NULL => {
            // log_error(null, "cannot print a NULL node");
        }
        _ => {
            println!(
                "\n LIB - unrecognized value type passed to println(). got type {:?}",
                visited_ast.r#type
            );
            std::process::exit(1);
        }
    }

    io::stdout().flush().unwrap();
}

#[no_mangle]
pub extern "C" fn lib_exit(visited_ast: &mut AST_T) {
    std::process::exit(visited_ast.int_value);
}

#[no_mangle]
pub extern "C" fn lib_strcat(left_ast: &mut AST_T, right_ast: &mut AST_T) -> *mut i8 {
    let left = unsafe { CStr::from_ptr(left_ast.string_value) }
        .to_owned()
        .into_string()
        .unwrap();

    let right = unsafe { CStr::from_ptr(right_ast.string_value) }
        .to_owned()
        .into_string()
        .unwrap();

    let con = left + &right;

    return CString::new(con).unwrap().into_raw();
}

#[no_mangle]
pub extern "C" fn lib_read_file_to_string(visited_ast: &mut AST_T) -> *mut i8 {
    let file_path = unsafe { CStr::from_ptr(visited_ast.string_value) }
        .to_owned()
        .into_string()
        .unwrap();

    let file_content = std::fs::read_to_string(file_path).unwrap();

    return CString::new(file_content).unwrap().into_raw();
}

#[no_mangle]
pub extern "C" fn lib_write_string_to_file(path_node: &mut AST_T, content_node: &mut AST_T) {
    let file_path = unsafe { CStr::from_ptr(path_node.string_value) }
        .to_owned()
        .into_string()
        .unwrap();

    let content = unsafe { CStr::from_ptr(content_node.string_value) }
        .to_owned()
        .into_string()
        .unwrap();

    std::fs::write(file_path, content).unwrap();
}

#[no_mangle]
pub extern "C" fn lib_pwd() -> *mut i8 {
    match std::env::current_dir() {
        Ok(current_dir) => {
            let res = current_dir.to_str().unwrap();
            return CString::new(res).unwrap().into_raw();
        }
        Err(e) => {
            println!("Error getting current working directory: {}", e);
            std::process::exit(1);
        }
    }
}

#[no_mangle]
pub extern "C" fn lib_get_timestamp() -> c_int {
    let now = SystemTime::now();
    let unix_timestamp = now
        .duration_since(UNIX_EPOCH)
        .expect("Time went backwards")
        .as_millis();

    let timestamp = c_int::from(unix_timestamp as i32);

    return timestamp;
}

#[no_mangle]
pub extern "C" fn lib_cmd(visited_ast: &mut AST_T) {
    let shell_command = unsafe { CStr::from_ptr(visited_ast.string_value) }
        .to_owned()
        .into_string()
        .unwrap();

    let mut child = Command::new("sh")
        .arg("-c")
        .arg(shell_command)
        .stdout(Stdio::piped())
        .stderr(Stdio::piped())
        .spawn()
        .expect("Failed to start command");

    let stdout = BufReader::new(child.stdout.take().unwrap());
    let stderr = BufReader::new(child.stderr.take().unwrap());

    let stdout_lines = stdout.lines();
    let stderr_lines = stderr.lines();

    let mut output = io::LineWriter::new(io::stdout());

    for line in stdout_lines.chain(stderr_lines) {
        if let Ok(line) = line {
            writeln!(output, "{}", line).expect("Failed to write to stdout");
            io::stdout().flush().expect("Failed to flush stdout");
        } else {
            break;
        }
    }

    let status = child.wait().expect("Failed to wait for command");
    if !status.success() {
        eprintln!("Command failed with status: {}", status);
    }
}

#[no_mangle]
pub extern "C" fn lib_chad_run(visited_ast: &mut AST_T) {
    let shell_command = unsafe { CStr::from_ptr(visited_ast.string_value) }
        .to_owned()
        .into_string()
        .unwrap();

    let mut child = Command::new("sh")
        .arg("-c")
        .arg("chad run ".to_string() + &shell_command)
        .stdout(Stdio::piped())
        .stderr(Stdio::piped())
        .spawn()
        .expect("Failed to start command");

    let stdout = BufReader::new(child.stdout.take().unwrap());
    let stderr = BufReader::new(child.stderr.take().unwrap());

    let stdout_lines = stdout.lines();
    let stderr_lines = stderr.lines();

    let mut output = io::LineWriter::new(io::stdout());

    for line in stdout_lines.chain(stderr_lines) {
        if let Ok(line) = line {
            writeln!(output, "{}", line).expect("Failed to write to stdout");
            io::stdout().flush().expect("Failed to flush stdout");
        } else {
            break;
        }
    }

    let status = child.wait().expect("Failed to wait for command");
    if !status.success() {
        eprintln!("Command failed with status: {}", status);
    }
}

// #[no_mangle]
// pub extern "C" fn std_println<'a>(
//     visitor: &'a mut visitor_T,
//     fdef: &'a mut AST_T,
// ) -> &'a mut AST_T {
//     let size_ast = unsafe {
//         scope_get_variable_definition(
//             visitor.tracker,
//             (*fdef.function_definition_body).scope,
//             "__vars__size",
//         )
//     };

//     let size = size_ast.variable_definition_value.int_value;

//     for i in 0..size {
//         let mut arg_name = String::from("__vars__");

//         let index_str = i.to_string();
//         arg_name.push_str(&index_str);

//         let vdef = unsafe {
//             scope_get_variable_definition(
//                 visitor.tracker,
//                 (*fdef.function_definition_body).scope,
//                 &arg_name,
//             )
//         };

//         let visited_ast = unsafe { visit(visitor, (*vdef).variable_definition_value) };

//         match (*visited_ast).r#type {
//             ast_type_t::AST_STRING => {
//                 print!("{}", visited_ast.string_value.unwrap());
//             }
//             ast_type_t::AST_CHAR => {
//                 print!("{}", visited_ast.char_value.unwrap());
//             }
//             ast_type_t::AST_INT => {
//                 print!("{}", visited_ast.int_value.unwrap());
//             }
//             ast_type_t::AST_FLOAT => {
//                 print!("{}", visited_ast.float_value.unwrap());
//             }
//             ast_type_t::AST_BOOL => {
//                 if visited_ast.bool_value.unwrap() {
//                     print!("true");
//                 } else {
//                     print!("false");
//                 }
//             }
//             ast_type_t::AST_NULL => {
//                 log_error(visitor.tracker, "cannot print a NULL node");
//             }
//             _ => {
//                 println!(
//                     "\n LIB - unrecognized value type passed to println(). got type {:?}",
//                     visited_ast.r#type
//                 );
//                 print!("{:p}", visited_ast);
//             }
//         }
//     }

//     println!("\n");

//     fdef
// }
