#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "rpc_parser.h"
#include "rpc_codegen.h"

static const char* type2type1(const char* name)
{
	if(strcmp(name, "_U8")==0)			return "_U8";
	if(strcmp(name, "_U16")==0)			return "_U16";
	if(strcmp(name, "_U32")==0)			return "_U32";
	if(strcmp(name, "_U64")==0)			return "_U64";
	if(strcmp(name, "_S8")==0)			return "_S8";
	if(strcmp(name, "_S16")==0)			return "_S16";
	if(strcmp(name, "_S32")==0)			return "_S32";
	if(strcmp(name, "_S64")==0)			return "_S64";
	if(strcmp(name, "_F32")==0)			return "_F32";
	if(strcmp(name, "_F64")==0)			return "_F64";

	return name;
}

static int type_isbasetype(const char* name)
{
	if(strcmp(name, "_U8")==0)			return 1;
	if(strcmp(name, "_U16")==0)			return 1;
	if(strcmp(name, "_U32")==0)			return 1;
	if(strcmp(name, "_U64")==0)			return 1;
	if(strcmp(name, "_S8")==0)			return 1;
	if(strcmp(name, "_S16")==0)			return 1;
	if(strcmp(name, "_S32")==0)			return 1;
	if(strcmp(name, "_S64")==0)			return 1;
	if(strcmp(name, "_F32")==0)			return 1;
	if(strcmp(name, "_F64")==0)			return 1;
	return 0;
}

int code_check()
{
	return 1;
}

int code_gen_clt_inc(const char* name, FILE* fp)
{
	unsigned int i, c, a;

	fprintf(fp, "////////////////////////////////////////////////\n");
	fprintf(fp, "// THIS FILE IS AUTOGENERATED. DO NOT MODIFY! //\n");
	fprintf(fp, "////////////////////////////////////////////////\n");
	fprintf(fp, "\n");
	fprintf(fp, "#ifndef __%s__CLIENT_STUB__\n", name);
	fprintf(fp, "#define __%s__CLIENT_STUB__\n", name);
	fprintf(fp, "\n");

	for(i=0; i<rpcgen_inc_count(); i++) {
		fprintf(fp, "#include \"%s\"\n", rpcgen_inc_get(i));
	}
	fprintf(fp, "\n");

	for(i=0; i<rpcgen_getcount(); i++) {
		const RPC_CLS* cls = rpcgen_get(i);
		for(c=0; c<cls->cmds_count; c++) {
			const RPC_CMD* cmd = cls->cmds + c;
			if(strcmp(cmd->mode, "message")==0) {
				fprintf(fp, "Zion::RPC_RESULT %s(Zion::HSERVER hServer", cmd->name);
			} else if(strcmp(cmd->mode, "callback")==0) {
				fprintf(fp, "void %s(Zion::HSERVER hServer", cmd->name);
			} else {
				continue;
			}
			for(a=0; a<cmd->args_count; a++) {
				fprintf(fp, ", ");
				if(cmd->args[a].size[0]=='\0') {
					if(type_isbasetype(cmd->args[a].type)) {
						fprintf(fp, "%s %s", type2type1(cmd->args[a].type), cmd->args[a].name);
					} else {
						fprintf(fp, "const %s& %s", type2type1(cmd->args[a].type), cmd->args[a].name);
					}
				} else {
					fprintf(fp, "const %s* %s", type2type1(cmd->args[a].type), cmd->args[a].name);
				}
			}
			fprintf(fp, ");\n");
		}
	}
	fprintf(fp, "\n");

	fprintf(fp, "#endif\n");
	return 1;
}

int code_gen_clt_src(const char* name, FILE* fp)
{
	unsigned int i, c, a;

	fprintf(fp, "////////////////////////////////////////////////\n");
	fprintf(fp, "// THIS FILE IS AUTOGENERATED. DO NOT MODIFY! //\n");
	fprintf(fp, "////////////////////////////////////////////////\n");
	fprintf(fp, "\n");
	fprintf(fp, "#include <ZionDefines.h>\n");
	fprintf(fp, "#include <AsyncRPC.h>\n");
	fprintf(fp, "#include <AsyncRPCImpl.h>\n");
	fprintf(fp, "#include \"%s.c.h\"\n", name);
	fprintf(fp, "\n");
	for(i=0; i<rpcgen_getcount(); i++) {
		fprintf(fp, "static void %s_Client_Register();\n", rpcgen_get(i)->name);
	}
	fprintf(fp, "\n");

	for(i=0; i<rpcgen_getcount(); i++) {
		int call_count = 0;
		const RPC_CLS* cls = rpcgen_get(i);
		for(c=0; c<cls->cmds_count; c++) {
			const RPC_CMD* cmd = cls->cmds + c;

			if(strcmp(cmd->mode, "message")==0) {
				fprintf(fp, "Zion::RPC_RESULT %s(Zion::HSERVER hServer", cmd->name);
				for(a=0; a<cmd->args_count; a++) {
					fprintf(fp, ", ");
					if(cmd->args[a].size[0]=='\0') {
						if(type_isbasetype(cmd->args[a].type)) {
							fprintf(fp, "%s %s", type2type1(cmd->args[a].type), cmd->args[a].name);
						} else {
							fprintf(fp, "const %s& %s", type2type1(cmd->args[a].type), cmd->args[a].name);
						}
					} else {
						fprintf(fp, "const %s* %s", type2type1(cmd->args[a].type), cmd->args[a].name);
					}
				}
				fprintf(fp, ")\n");
				fprintf(fp, "{\n");
				fprintf(fp, "	%s_Client_Register();\n", cls->name);

				for(a=0; a<cmd->args_count; a++) {
					if(cmd->args[a].size[0]!='\0') {
						fprintf(fp, "	_U32 %s__length = (_U32)(%s);\n", cmd->args[a].name, cmd->args[a].size);
					}
				}

				fprintf(fp, "	Zion::RPC_OUTPUT_BUF buf(RPC_PACKET_OVERHEAD");
				for(a=0; a<cmd->args_count; a++) {
					if(cmd->args[a].size[0]=='\0') {
						fprintf(fp, "+_aligned_sizeof(%s)", cmd->args[a].name);
					} else {
						fprintf(fp, "+_aligned_sizeof(%s__length)+_buffer_alignof(sizeof(%s[0])*%s__length)", cmd->args[a].name, cmd->args[a].name, cmd->args[a].name);
					}
				}
				fprintf(fp, ");\n");

				fprintf(fp, "	if(!buf) return(RPC_RES_OOM);\n");
				for(a=0; a<cmd->args_count; a++) {
					if(cmd->args[a].size[0]=='\0') {
						fprintf(fp, "	if(!buf.Serialize(%s)) return(RPC_RES_TMP);\n", cmd->args[a].name);
					} else {
						fprintf(fp, "	if(!buf.Serialize(%s__length)) return(RPC_RES_TMP);\n", cmd->args[a].name);
						fprintf(fp, "	if(!buf.Serialize(%s__length, %s)) return(RPC_RES_TMP);\n", cmd->args[a].name, cmd->args[a].name);
					}
				}
				fprintf(fp, "	buf.SetID(%s, %d);\n", cls->number, call_count);
				fprintf(fp, "	return(buf.SendTo(hServer));\n");
				fprintf(fp, "}\n");
				fprintf(fp, "\n");
				call_count++;
				continue;
			}
			if(strcmp(cmd->mode, "callback")==0) {

				fprintf(fp, "static bool %s_stub_%d(Zion::HSERVER hServer, Zion::RPC_INPUT_BUF& buf)\n", cmd->name, cmd->index);
				fprintf(fp, "{\n");
				for(a=0; a<cmd->args_count; a++) {
					if(cmd->args[a].size[0]=='\0') {
						fprintf(fp, "	%s* %s;\n", type2type1(cmd->args[a].type), cmd->args[a].name);
					} else {
						fprintf(fp, "	%s* %s;\n", type2type1(cmd->args[a].type), cmd->args[a].name);
						fprintf(fp, "	unsigned int* %s__length;\n", cmd->args[a].name);
					}
				}
				for(a=0; a<cmd->args_count; a++) {
					if(cmd->args[a].size[0]=='\0') {
						fprintf(fp, "	if(!buf.Serialize(%s)) return false;\n", cmd->args[a].name);
					} else {
						fprintf(fp, "	if(!buf.Serialize(%s__length)) return false;\n", cmd->args[a].name);
						fprintf(fp, "	if(!buf.Serialize(*%s__length, %s)) return false;\n", cmd->args[a].name, cmd->args[a].name);
					}
				}
				fprintf(fp, "	%s(hServer", cmd->name);
				for(a=0; a<cmd->args_count; a++) {
					fprintf(fp, ", ");
					if(cmd->args[a].size[0]=='\0') {
						fprintf(fp, "*%s", cmd->args[a].name);
					} else {
						fprintf(fp, "%s", cmd->args[a].name);
					}
				}
				fprintf(fp, ");\n");
				fprintf(fp, "	return true;\n");
				fprintf(fp, "}\n");
				fprintf(fp, "\n");
				continue;
			}
		}
	}

	for(i=0; i<rpcgen_getcount(); i++) {
		const RPC_CLS* cls = rpcgen_get(i);
		fprintf(fp, "	static Zion::RPC_CBFUNC_STUB %s_callback_table[] = {\n", cls->name);
		for(c=0; c<cls->cmds_count; c++) {
			const RPC_CMD* cmd = cls->cmds + c;
			if(strcmp(cmd->mode, "callback")==0) {
				fprintf(fp, "	%s_stub_%d,\n", cmd->name, cmd->index);
			}
		}
		fprintf(fp, "	NULL};\n");
		fprintf(fp, "void %s_Client_Register() {\n", cls->name);
		fprintf(fp, "	Zion::lwrpc_cbinterface_table[%s].cbfcount = sizeof(%s_callback_table)/sizeof(%s_callback_table[0]) - 1;\n", cls->number, cls->name, cls->name);
		fprintf(fp, "	Zion::lwrpc_cbinterface_table[%s].cbfunc_tbl = %s_callback_table;\n", cls->number, cls->name);
		fprintf(fp, "}\n");
	}
	return 1;
}

int code_gen_svr_inc(const char* name, FILE* fp)
{
	unsigned int i, c, a;

	fprintf(fp, "////////////////////////////////////////////////\n");
	fprintf(fp, "// THIS FILE IS AUTOGENERATED. DO NOT MODIFY! //\n");
	fprintf(fp, "////////////////////////////////////////////////\n");
	fprintf(fp, "\n");
	fprintf(fp, "#ifndef __%s__SERVER_STUB___\n", name);
	fprintf(fp, "#define __%s__SERVER_STUB___\n", name);
	fprintf(fp, "\n");

	for(i=0; i<rpcgen_inc_count(); i++) {
		fprintf(fp, "#include \"%s\"\n", rpcgen_inc_get(i));
	}
	fprintf(fp, "\n");

	for(i=0; i<rpcgen_getcount(); i++) {
		const RPC_CLS* cls = rpcgen_get(i);
		for(c=0; c<cls->cmds_count; c++) {
			const RPC_CMD* cmd = cls->cmds + c;

			if(strcmp(cmd->mode, "message")==0) {
				fprintf(fp, "void %s(Zion::HCLIENT hClient", cmd->name);
			} else if(strcmp(cmd->mode, "callback")==0) {
				fprintf(fp, "Zion::RPC_RESULT %s(Zion::HCLIENT hClient", cmd->name);
			} else {
				continue;
			}
			for(a=0; a<cmd->args_count; a++) {
				fprintf(fp, ", ");
				if(cmd->args[a].size[0]=='\0') {
					if(type_isbasetype(cmd->args[a].type)) {
						fprintf(fp, "%s %s", type2type1(cmd->args[a].type), cmd->args[a].name);
					} else {
						fprintf(fp, "const %s& %s", type2type1(cmd->args[a].type), cmd->args[a].name);
					}
				} else {
					fprintf(fp, "const %s* %s", type2type1(cmd->args[a].type), cmd->args[a].name);
				}
			}
			fprintf(fp, ");\n");
		}
	}
	fprintf(fp, "\n");
	for(i=0; i<rpcgen_getcount(); i++) {
		fprintf(fp, "void %s_Server_Register();\n", rpcgen_get(i)->name );
	}
	fprintf(fp, "\n");
	fprintf(fp, "#endif\n");
	return 1;
}

int code_gen_svr_src(const char* name, FILE* fp)
{
	unsigned int i, c, a;

	fprintf(fp, "////////////////////////////////////////////////\n");
	fprintf(fp, "// THIS FILE IS AUTOGENERATED. DO NOT MODIFY! //\n");
	fprintf(fp, "////////////////////////////////////////////////\n");
	fprintf(fp, "\n");
	fprintf(fp, "#include <ZionBase.h>\n");
	fprintf(fp, "#include <AsyncRPC.h>\n");
	fprintf(fp, "#include <AsyncRPCImpl.h>\n");
	fprintf(fp, "#include \"%s.s.h\"\n", name);
	fprintf(fp, "\n");

	for(i=0; i<rpcgen_getcount(); i++) {
		int call_count = 0;
		const RPC_CLS* cls = rpcgen_get(i);
		for(c=0; c<cls->cmds_count; c++) {
			const RPC_CMD* cmd = cls->cmds + c;
			if(strcmp(cmd->mode, "message")==0) {
				fprintf(fp, "static bool %s_stub_%d(Zion::HCLIENT hClient, Zion::RPC_INPUT_BUF& buf)\n", cmd->name, cmd->index);
				fprintf(fp, "{\n");
				for(a=0; a<cmd->args_count; a++) {
					if(cmd->args[a].size[0]=='\0') {
						fprintf(fp, "	%s* %s;\n", type2type1(cmd->args[a].type), cmd->args[a].name);
					} else {
						fprintf(fp, "	%s* %s;\n", type2type1(cmd->args[a].type), cmd->args[a].name);
						fprintf(fp, "	_U32* %s__length;\n", cmd->args[a].name);
					}
				}

				for(a=0; a<cmd->args_count; a++) {
					if(cmd->args[a].size[0]=='\0') {
						fprintf(fp, "	if(!buf.Serialize(%s)) return false;\n", cmd->args[a].name);
					} else {
						fprintf(fp, "	if(!buf.Serialize(%s__length)) return false;\n", cmd->args[a].name);
						fprintf(fp, "	if(!buf.Serialize(*%s__length, %s)) return false;\n", cmd->args[a].name, cmd->args[a].name);
					}
				}
				fprintf(fp, "	%s(hClient", cmd->name);
				for(a=0; a<cmd->args_count; a++) {
					fprintf(fp, ", ");
					if(cmd->args[a].size[0]=='\0') {
						fprintf(fp, "*%s", cmd->args[a].name);
					} else {
						fprintf(fp, "%s", cmd->args[a].name);
					}
				}
				fprintf(fp, ");\n");
				fprintf(fp, "	return true;\n");
				fprintf(fp, "}\n");
				fprintf(fp, "\n");
				continue;
			}
			if(strcmp(cmd->mode, "callback")==0) {
				fprintf(fp, "Zion::RPC_RESULT %s(Zion::HCLIENT hClient", cmd->name);
				for(a=0; a<cmd->args_count; a++) {
					fprintf(fp, ", ");
					if(cmd->args[a].size[0]=='\0') {
						if(type_isbasetype(cmd->args[a].type)) {
							fprintf(fp, "%s %s", type2type1(cmd->args[a].type), cmd->args[a].name);
						} else {
							fprintf(fp, "const %s& %s", type2type1(cmd->args[a].type), cmd->args[a].name);
						}
					} else {
						fprintf(fp, "const %s* %s", type2type1(cmd->args[a].type), cmd->args[a].name);
					}
				}
				fprintf(fp, ")\n");
				fprintf(fp, "{\n");
				for(a=0; a<cmd->args_count; a++) {
					if(cmd->args[a].size[0]!='\0') {
						fprintf(fp, "	unsigned int %s__length = (_U32)(%s);", cmd->args[a].name, cmd->args[a].size);
					}
				}

				fprintf(fp, "	Zion::RPC_OUTPUT_BUF buf(RPC_PACKET_OVERHEAD");
				for(a=0; a<cmd->args_count; a++) {
					if(cmd->args[a].size[0]=='\0') {
						fprintf(fp, "+_aligned_sizeof(%s)", cmd->args[a].name);
					} else {
						fprintf(fp, "+_aligned_sizeof(%s__length)+_buffer_alignof(sizeof(%s[0])*%s__length)", cmd->args[a].name, cmd->args[a].name, cmd->args[a].name);
					}
				}
				fprintf(fp, ");\n");

				fprintf(fp, "	if(!buf) return(RPC_RES_OOM);\n");
				for(a=0; a<cmd->args_count; a++) {
					if(cmd->args[a].size[0]=='\0') {
						fprintf(fp, "	if(!buf.Serialize(%s)) return(RPC_RES_TMP);\n", cmd->args[a].name);
					} else {
						fprintf(fp, "	if(!buf.Serialize(%s__length)) return(RPC_RES_TMP);\n", cmd->args[a].name);
						fprintf(fp, "	if(!buf.Serialize(%s__length, %s)) return(RPC_RES_TMP);\n", cmd->args[a].name, cmd->args[a].name);
					}
				}
				fprintf(fp, "	buf.SetID(%s, %d);\n", cls->number, call_count);
				fprintf(fp, "	return(buf.SendTo(hClient));\n");
				fprintf(fp, "}\n");
				fprintf(fp, "\n");
				call_count++;
				continue;
			}
		}
	}

	for(i=0; i<rpcgen_getcount(); i++)
	{
		const RPC_CLS* cls = rpcgen_get(i);
		fprintf(fp, "static Zion::RPC_FUNC_STUB %s_server_table[] =\n", cls->name);
		fprintf(fp, "{\n");
		for(c=0; c<cls->cmds_count; c++) {
			const RPC_CMD* cmd = cls->cmds + c;
			if(strcmp(cmd->mode, "message")==0) {
				fprintf(fp, "%s_stub_%d,\n", cmd->name, cmd->index);
			}
		}
		fprintf(fp, "NULL };\n");
		fprintf(fp, "void %s_Server_Register() {\n", cls->name );
		fprintf(fp, "	Zion::lwrpc_interface_table[%s].fcount = sizeof(%s_server_table)/sizeof(%s_server_table[0]) - 1;\n", cls->number, cls->name, cls->name);
		fprintf(fp, "	Zion::lwrpc_interface_table[%s].func_tbl = %s_server_table;\n", cls->number, cls->name);
		fprintf(fp, "}\n");
	}

	return 1;
}