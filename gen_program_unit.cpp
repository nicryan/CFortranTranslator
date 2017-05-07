/*
*   Calvin Neo
*   Copyright (C) 2016  Calvin Neo <calvinneo@calvinneo.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License along
*   with this program; if not, write to the Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "gen_common.h"

/*
R202 program-unit is main-program
	or external-subprogram
	or module
	or block-data
*/
// external-subprogram is in gen_function


void gen_fortran_program(ARG_IN wrappers) {
	std::string codes;
	std::string main_code;
	get_context().program_tree = wrappers;
	FunctionInfo * program_info = add_function("", "program", FunctionInfo());
	ParseNode script_program = gen_token(Term{ TokenMeta::NT_SUITE , "" });
	for (int i = 0; i < wrappers.length(); i++)
	{
		// gather all TokenMeta::SUITE
		const ParseNode & wrapper = wrappers.get(i);
		if (wrapper.get_token() == TokenMeta::NT_SUITE)
		{
			for (int j = 0; j < wrapper.length(); j++)
			{
				script_program.addchild(wrapper.get(j));
			}
		}
	}
	get_context().current_module = "";
	regen_suite(program_info, script_program, true);
	main_code += tabber(script_program.get_what());
	for (int i = 0; i < wrappers.length(); i++)
	{
		ParseNode & wrapper = get_context().program_tree.get(i);
		if (wrapper.get_token() == TokenMeta::NT_PROGRAM_EXPLICIT)
		{
			get_context().current_module = "";
			regen_suite(program_info, wrapper.get(0));
			main_code += tabber(wrapper.get(0).get_what());
		}
		else if (wrapper.get_token() == TokenMeta::NT_FUNCTIONDECLARE) {
			get_context().current_module = "";
			ParseNode & variable_function = wrapper.get(1);
			FunctionInfo * finfo = add_function(get_context().current_module, variable_function.get_what(), FunctionInfo{});
			regen_function(finfo, wrapper);
			codes += wrapper.get_what();
			codes += "\n";
		}
		else if (wrapper.get_token() == TokenMeta::NT_SUITE)
		{
			// handled in the previous loop
			continue;
		}
		else if (wrapper.get_token() == TokenMeta::NT_DUMMY)
		{
			// YY_END
		}
		else {
			print_error("Unexpected wrappers");
		}
	}
	sprintf(codegen_buf, "int main()\n{\n%s\treturn 0;\n}", main_code.c_str());
	codes += string(codegen_buf);
	// common definition
	for (std::map<std::string, CommonBlockInfo>::iterator iter = get_context().commonblocks.begin(); iter != get_context().commonblocks.end(); iter++)
	{
		string s = gen_common_definition(iter->first).get_what();
		codes = s + codes;
	}
	get_context().program_tree.get_what() = codes;
}