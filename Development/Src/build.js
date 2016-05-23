
var fs = require('fs');
var path = require('path');
var util = require('util');
var os = require('os');
var spawn = require('child_process').spawn;

if(!String.prototype.trim) {
	String.prototype.trim = function () {
		return this.replace(/^\s+|\s+$/g, '');
	};
}

if(!String.prototype.ltrim) {
	String.prototype.ltrim = function () {
		return this.replace(/^\s+/, '');
	};
}

if(!String.prototype.rtrim) {
	String.prototype.rtrim = function () {
		return this.replace(/\s+$/, '');
	};
}

if(!String.prototype.fulltrim) {
	String.prototype.fulltrim = function () {
		return this.replace(/(?:(?:^|\n)\s+|\s+(?:$|\n))/g,'').replace(/\s+/g, ' ');
	};
}

if(!String.prototype.format) {
	String.prototype.format = function() {
		var args = arguments;
		return this.replace(/{(\d+)}/g, function(match, number) {
			return typeof args[number] != 'undefined' ? args[number] : match ;
		});
	};
}

function mkdirp_sync (p, mode, made) {
	if(mode === undefined) {
		mode = 0777 & (~process.umask());
	}
	if(!made) made = null;

	if(typeof mode === 'string') mode = parseInt(mode, 8);
	p = path.resolve(p);

	try {
		fs.mkdirSync(p, mode);
		made = made || p;
	}
	catch (err0) {
		switch (err0.code) {
		case 'ENOENT' :
			made = mkdirp_sync(path.dirname(p), mode, made);
			mkdirp_sync(p, mode, made);
			break;

		// In the case of any other error, just see if there's a dir
		// there already.  If so, then hooray!  If not, then something
		// is borked.
		default:
			var stat;
			try {
				stat = fs.statSync(p);
			}
			catch (err1) {
				throw err0;
			}
			if(!stat.isDirectory()) throw err0;
			break;
		}
	}

	return made;
};

function execute(exe_file, args, callback) {
	var stdout_buffer = "";
	var stderr_buffer = "";
	var exe_proc;
	try {
		exe_proc = spawn(exe_file, args);
	} catch(e) {
		console.log('failed to spawn process.');
		console.log('  ' + e.message);
		console.log('  ' + exe_file, args.join(' '));
		callback(-1);
		return;
	}
	console.log('', exe_proc.pid, ":", 'process spwned :', exe_file, args.join(' '));
	exe_proc.stdout.on('data', function (data) {
		stdout_buffer += data;
		for(;;) {
			var pos = stdout_buffer.indexOf('\n');
			if(pos<=0) break;
			console.log('', exe_proc.pid, ":", stdout_buffer.substring(0, pos));
			stdout_buffer = stdout_buffer.substring(pos+1);
		}
	});
	exe_proc.stderr.on('data', function (data) {
		stderr_buffer += data;
		for(;;) {
			var pos = stderr_buffer.indexOf('\n');
			if(pos<=0) break;
			console.log('', exe_proc.pid, ':', stderr_buffer.substring(0, pos));
			stderr_buffer = stderr_buffer.substring(pos+1);
		}
	});
	exe_proc.on('close', function (code) {
		if(stdout_buffer!="") console.log(stdout_buffer);
		if(stderr_buffer!="") console.log(stderr_buffer);
		console.log('', exe_proc.pid, ":", 'process exited, code =', code);
		callback(code);
	});
}

function Task(exe, args, outs, deps) {
	this.task_exe = exe;
	this.task_args = args;
	this.out_files = outs;
	this.dep_files = deps;
	this.manager = undefined;
}

Task.prototype.emit = function (dep_file) {
	if(dep_file) {
		var pos = this.dep_files.indexOf(dep_file);
		if(pos>=0) {
			this.dep_files[pos] = '';
		}
	}

	for(var i=0; i<this.dep_files.length; i++) {
		if(this.dep_files[i]!='') return;
	}

	this.manager.ready(this);
};

function TaskManager(slot_max) {
	this.tasks = [];
	this.ready_tasks = [];
	this.files = {};
	this.slot_max = slot_max;
	this.slot_used = 0;
	this.callback = undefined;
	this.has_error = false;
}

TaskManager.prototype.reset = function () {
	this.tasks = [];
	this.ready_tasks = [];
	this.files = {};
	this.slot_used = 0;
	this.callback = undefined;
	this.has_error = false;
};

TaskManager.prototype.addTask = function (exe, args, outs, deps) {
	var task = new Task(exe, args, outs, deps);
	task.manager = this;
	this.tasks.push(task);
};

TaskManager.prototype.ready = function (task) {
	this.ready_tasks.push(task);
};

TaskManager.prototype.build = function () {
	for(var t=0; t<this.tasks.length; t++) {
		var task = this.tasks[t];
		for(var i=0; i<task.out_files.length; i++) {
			var out_file = task.out_files[i];
			if(this.files.hasOwnProperty(out_file)) {
				console.log('outfile already existed.', out_file);
			} else {
				item = [];
				this.files[out_file] = item;
			}
		}
	}

	for(var t=0; t<this.tasks.length; t++) {
		var task = this.tasks[t];
		for(var i=0; i<task.dep_files.length; i++) {
			var dep_file = task.dep_files[i];
			if(this.files.hasOwnProperty(dep_file)) {
				var item = this.files[dep_file];
				item.push(task);
			} else {
				task.dep_files[i] = '';
			}
		}
	}
};

TaskManager.prototype.showDepends = function () {
	for(var t=0; t<this.tasks.length; t++) {
		var task = this.tasks[t];
		console.log(task.out_files.join(' '), ':', task.dep_files.join(' '));
	}
};

TaskManager.prototype.emit = function (out_file) {
	if(this.files.hasOwnProperty(out_file)) {
		var tasks = this.files[out_file];
		for(var t=0; t<tasks.length; t++) {
			tasks[t].emit(out_file);
			delete this.files[out_file];
		}
	}
};

TaskManager.prototype.execute = function (task) {
	this.slot_used += 1;
	var manager = this;
	execute(task.task_exe, task.task_args, function(code) {
		manager.slot_used -= 1;
		if(code!=0) {
			manager.has_error = true;
		} else {
			for(var i=0; i<task.out_files.length; i++) {
				manager.emit(task.out_files[i]);
			}
		}
		manager.schedule();
	});
};

TaskManager.prototype.schedule = function () {
	while(this.slot_used<this.slot_max && this.ready_tasks.length>0) {
		var task = this.ready_tasks.shift();
		this.execute(task);
	}

	if(this.ready_tasks.length==0 && this.slot_used==0) {
		var callback = this.callback;
		this.callback = undefined;
		callback(this.has_error);
	}
};

TaskManager.prototype.start = function (callback) {
	this.callback = callback;
	this.has_error = false;

	for(var t=0; t<this.tasks.length; t++) {
		this.tasks[t].emit(undefined);
	}

	this.schedule();
};

function deleteFile(filename) {
	if(fs.existsSync(filename)) {
		fs.unlinkSync(filename);
	}
}

function deleteFiles(files) {
	for(var i=0; i<files.length; i++) {
		deleteFile(files[i]);
	}
}

function getFileExt(file) {
	var pos = file.lastIndexOf('.');
	if(pos<=0) return '';
	return file.substring(pos+1);
}

function ProcessPropertyFile (proj, filename) {
	var path = proj.path + filename;
	path = path.replace(/\\/g, "/");
	var lines = fs.readFileSync(path, 'utf-8').split('\n');

	for(var i=0; i<lines.length; i++) {
		var pos;
		pos = lines[i].indexOf('<AdditionalIncludeDirectories');
		if(pos>0) {
			pos = lines[i].indexOf('>');
			var value = lines[i].substring(pos+1);
			value = value.substring(0, value.indexOf('<'));
			value = value.replace(/\\/g, "/");
			value = value.replace(/\;\%\(AdditionalIncludeDirectories\)/g, "/");
			value = value.split(';');
			for(i=0; i<value.length; i++) {
				if(value[i].indexOf("$(SolutionDir)")>=0) {
					value[i] = value[i].replace(/\$\(SolutionDir\)/g, "");
				} else {
					value[i] = proj.path + value[i];
				}
			}
			return value;
		}
	}
	return [];
}

function ProjectFile () {
	this.path = '';
	this.name = '';
	this.deps = [];
	this.type = '';
	this.inc_files = [];
	this.ddl_files = [];
	this.rpc_files = [];
	this.src_files = [];
	this.inc_dir = [];
	this.ext_deps = [];
}

ProjectFile.prototype.load = function (filename) {
	this.path = filename.substring(0, filename.lastIndexOf('\\')+1);
	this.name = filename.substring(filename.lastIndexOf('\\')+1);
	this.name = this.name.substring(0, this.name.lastIndexOf('.'));

	filename = filename.replace(/\\/g, "/");

	var lines = fs.readFileSync(path.normalize(filename), 'utf-8').split('\n');

	for(var i=0; i<lines.length; i++) {
		var pos;

		pos = lines[i].indexOf('<Import Project="');
		if(pos>0) {
			pos = lines[i].indexOf('"');
			var value = lines[i].substring(pos+1);
			value = value.substring(0, value.indexOf('"'));
			if(value.indexOf('$')>=0) {
				continue;
			}
			pos = Math.max(value.lastIndexOf('/'), value.lastIndexOf('\\'));
			switch(value.substring(pos + 1)) {
			case 'Win32.props':
			    break;
			case 'wxWidgets.props':
				this.ext_deps.push("wxWidgets");
			    break;
			case 'mysql.props':
				this.ext_deps.push("mysql");
			    break;
            default:
                this.inc_dir = this.inc_dir.concat(ProcessPropertyFile(this, value));
            }
			continue;
		}

		pos = lines[i].indexOf('<ConfigurationType>');
		if(pos>0) {
			pos = lines[i].indexOf('>');
			var name = lines[i].substring(pos+1);
			name = name.substring(0, name.indexOf('<'));
			this.type = name;
			continue;
		}

		pos = lines[i].indexOf('<ProjectReference Include="');
		if(pos>0) {
			var name = lines[i];
			name = name.substring(name.lastIndexOf('\\')+1);
			name = name.substring(0, name.indexOf('.vcxproj'));
			this.deps.push(name);
			continue;
		}

		pos = lines[i].indexOf('<ClCompile Include="');
		var files;
		if(pos>=0) {
			files = this.src_files;
		} else {
			pos = lines[i].indexOf('<ClInclude Include="');
			if(pos>=0) {
				files = this.inc_files;
			} else {
				pos = lines[i].indexOf('<None Include="');
				if(pos>=0 && lines[i].indexOf('.m')>0) {
					files = this.src_files;
				} else {
					pos = lines[i].indexOf('<CustomBuild Include="');
					if(pos>=0) {
						pos = lines[i].indexOf('.ddl"');
						if(pos>=0) {
							files = this.ddl_files;
						} else {
							pos = lines[i].indexOf('.rpc"');
							if(pos>=0) {
								files = this.rpc_files;
							} else {
								continue;
							}
						}
					} else {
						continue;
					}
				}
			}
		}
		pos = lines[i].indexOf('"');
		if(pos<0) throw "wwwwww";
		var name = lines[i].substring(pos+1);
		pos = name.indexOf('"');
		if(pos<0) throw "wwwwww";
		name = name.substring(0, pos).replace(/\\/g, "/");
		files.push(name);
	}
}

function SolutionFile () {
	this.projs = [];
}

SolutionFile.prototype.load = function (filename) {
	var lines = fs.readFileSync(filename, 'utf-8').split('\n');

	for(var i=0; i<lines.length; i++) {
		var pos = lines[i].indexOf('Project("');
		if(pos<0) continue;
		pos = lines[i].indexOf(',');
		if(pos<0) throw "invalid solution file";
		var name = lines[i].substring(pos+3);
		pos = name.indexOf('"');
		if(pos<0) throw "invalid solution file";
		name = name.substring(0, pos);
		pos = name.indexOf('.vcxproj');
		if(pos<0) continue;
		this.projs.push(name);
	}

	for(var i=0; i<this.projs.length; i++) {
		var proj = new ProjectFile();
		proj.load(this.projs[i]);
		this.projs[i] = proj;
	}

	return true;
}

SolutionFile.prototype.getProject = function (name) {
	for(var i=0; i<this.projs.length; i++) {
		if(this.projs[i].name==name) {
			return this.projs[i];
		}
	}
	return undefined;
}

function AppBuilder (solution, manager) {
	this.files_data = {};
	this.solution = solution;
	this.jobs = [];
	this.ext_deps = {};
	this.manager = manager;
}

AppBuilder.prototype.addTask = function (cmd_line, outs, deps) {
	var pos = cmd_line.indexOf(' ');
	if(pos<0) pos = cmd_line.length;
	var task_exe = cmd_line.substring(0, pos);
	var task_arg = cmd_line.substring(pos+1).split(' ');

	if(task_exe=='echo') {
		throw new Exception('xxxxxxxx');
	}

	for(var i=0; i<outs.length; i++) {
		outs[i] = path.normalize(outs[i]);
	}
	for(var i=0; i<deps.length; i++) {
		deps[i] = path.normalize(deps[i]);
	}
	this.manager.addTask(task_exe, task_arg, outs, deps);
}

AppBuilder.prototype.writeLog = function () {
/*
	if(arguments.length>0) {
		var args = [''];
		for(var i=0; i<arguments.length; i++) {
			args.push(arguments[i]);
		}
		console.log.apply(undefined, args);
	} else {
		console.log('');
	}
*/
}

AppBuilder.prototype.getFileTime = function (filename) {
	if(filename in this.files_data) {
		return this.files_data[filename];
	} else {
		var ts;
		ts = Date.parse(fs.statSync(filename).mtime) / 1000;
		this.files_data[filename] = ts;
		return ts;
	}
}

AppBuilder.prototype.setBuild = function (platform, config)
{
	this.platform = platform;
	this.config = config;

	if(this.platform=='win32') {
		this.obj_ext = '.obj';
		this.exe_ext = '.exe';
		this.dll_ext = '.dll';
		this.lib_ext = '.lib';
		this.cd_exe = 'echo cl {1} {0} /c /Fo{2}';
		this.cc_exe = 'cl {1} {0} /c /Fo{2}';
		this.ld_exe = 'cl {2} {0} /Fe{1}';
		this.sl_exe = 'lib {2} -OUT:{0}{1}';
		this.dl_exe = '';
	} else {
		this.obj_ext = '.o';
		this.exe_ext = '';
		this.dll_ext = '.so';
		this.lib_ext = '.a';
		this.cd_exe = 'gcc -M {1} {0} -c -o {2}';
		this.cc_exe = 'clang++ {1} {0} -c -o {2}';
		this.ld_exe = 'clang++ {0} {2} -o {1}';
		this.sl_exe = 'ar rcs {0}lib{1} {2}';
		this.dl_exe = '';
	}

	if(config=='debug') {
		this.output_dir = path.resolve("./", "../..") + "/Binaries/Debug/";
		this.object_dir = path.resolve("./", "..") + "/Intermediate/Debug/";
		this.cc_flag = '-g -D__x86_64__';
		this.ln_flag = '';
		this.sl_flag = '';
		this.dl_flag = '';
        if(this.platform=='win32') {
			this.cc_flag = ' /MTd /Od -D__x86_64__';
			this.ln_flag = ' /MTd';
		}
	}
	if(config=='release') {
		this.output_dir = path.resolve("./", "../..") + "/Binaries/Release/";
		this.object_dir = path.resolve("./", "..") + "/Intermediate/Release/";
		this.cc_flag = '-O2 -D__x86_64__';
		this.ln_flag = '';
		this.sl_flag = '';
		this.dl_flag = '';
        if(this.platform=='win32') {
			this.cc_flag = ' /MT /O2 -D__x86_64__';
			this.ln_flag = ' /MT /LTCG';
		}
	}

	var cc_flag, ln_flag;
	switch(this.platform)
	{
	case 'win32':
		this.cc_flag += ' -D_WIN32';
		break;
	case 'linux':
		this.cc_flag += ' -D_LINUX';
		this.ln_flag += " -ldl -pthread-luuid -lrt";

		cc_flag += '';
		ln_flag += " -lmysqlclient";
		this.ext_deps["mysql"] = { 'cc_flag' : cc_flag, 'ln_flag' : ln_flag };
		break;
	case 'darwin':
		this.cc_flag += ' -D_DARWIN';

		cc_flag = ' -I/usr/local/include';
		ln_flag = " -lmysqlclient -L/usr/local/lib";
		this.ext_deps["mysql"] = { 'cc_flag' : cc_flag, 'ln_flag' : ln_flag };

		cc_flag += " -D_FILE_OFFSET_BITS=64 -DwxDEBUG_LEVEL=0 -DWXUSINGDLL -D__WXMAC__ -D__WXOSX__ -D__WXOSX_COCOA__"
		cc_flag += " -I/usr/local/lib/wx/include/osx_cocoa-unicode-3.0 -I/usr/local/include/wx-3.0";
		ln_flag = " -lwx_baseu-3.0 -lwx_osx_cocoau_core-3.0 -lwx_osx_cocoau_propgrid-3.0 -lwx_osx_cocoau_aui-3.0  -lwx_osx_cocoau_adv-3.0";
		ln_flag += " -framework IOKit -framework Carbon -framework Cocoa -framework AudioToolbox -framework System -framework OpenGL";
		this.ext_deps["wxWidgets"] = { 'cc_flag' : cc_flag, 'ln_flag' : ln_flag };

		break;
	case 'freebsd':
		this.cc_flag += ' -D_FREEBSD';
		this.ln_flag += " -ldl -pthread -luuid -lrt";

		cc_flag += '';
		ln_flag += " -lmysqlclient";
		this.ext_deps["mysql"] = { 'cc_flag' : cc_flag, 'ln_flag' : ln_flag };

		break;
	}
}

AppBuilder.prototype.needUpdate = function (src_files, gen_files) {
	if(typeof(src_files)=='string') {
		src_files = [src_files];
	}

	var src_ts = 0;
	var src_filename = "";
	for(var i=0; i<src_files.length; i++) {
		if(!fs.existsSync(src_files[i])) {
			this.writeLog('needUpdate : source file not existed', src_files[i]);
			return true;
		}
		var f_ts = fs.statSync(src_files[i]);
		var ts = Date.parse(f_ts.mtime) + f_ts.mtime.getMilliseconds();
		if(ts>src_ts) {
			src_filename = src_files[i];
			src_ts = ts;
		}
	}

	if(typeof(gen_files)=='string') {
		gen_files = [gen_files];
	}

	for(var i=0; i<gen_files.length; i++) {
		if(!fs.existsSync(gen_files[i])) {
			this.writeLog('needUpdate : not existd ', src_ts, gen_files[i]);
			return true;
		}
		var f_ts = fs.statSync(gen_files[i]);
		var ts = Date.parse(f_ts.mtime) + f_ts.mtime.getMilliseconds();
		if(ts < src_ts) {
			this.writeLog('needUpdate : dirty #', new Date(ts), new Date(src_ts), gen_files[i], src_filename);
			return true;
		}
	}

	return false;
}

AppBuilder.prototype.getCCFlags = function (proj) {
	var cc_flag = this.cc_flag;
	for(var i=0; i<proj.ext_deps.length; i++) {
		if(this.ext_deps.hasOwnProperty(proj.ext_deps[i])) {
			var item = this.ext_deps[proj.ext_deps[i]];
			cc_flag += item.cc_flag;
		}
	}
	return cc_flag;
}

AppBuilder.prototype.getLNFlags = function (proj) {
	var ln_flag = this.ln_flag;
	for(var i=0; i<proj.ext_deps.length; i++) {
		if(this.ext_deps.hasOwnProperty(proj.ext_deps[i])) {
			var item = this.ext_deps[proj.ext_deps[i]];
			ln_flag += item.ln_flag;
		}
	}
	return ln_flag;
}

AppBuilder.prototype.loadCPPDepend = function (filename) {
	try {
		if(!fs.existsSync(filename)) {
			return undefined;
		}
		var arr = fs.readFileSync(filename, 'utf-8').split('\n');
		for(var i=0; i<arr.length; i++) {
			if(i==0) {
					var pos = arr[0].lastIndexOf(':');
				if(pos>=0) {
					arr[0] = arr[0].substring(pos+1).trim();
				}
			}
			if(arr[i].length>0) {
				if(arr[i].substring(arr[i].length-1)=='\\') {
					arr[i] = arr[i].substring(0, arr[i].length - 1);
					arr[i] = arr[i].trim();
				}
			}
		}
		return arr.join(' ').split(' ').filter(function(elm){
			return elm.trim()!='';
		});
	} catch(e) {
		this.writeLog('loadCPPDepend', 'failed.', filename);
		return undefined;
	}
}

AppBuilder.prototype.buildRPC = function (proj) {
	if(proj.rpc_files.length==0) return;

	this.build('RpcGen');

	var rpc_exe = path.normalize(this.output_dir + 'RpcGen' + this.exe_ext);
	for(var i=0; i<proj.rpc_files.length; i++) {
		var rpc_path = path.normalize(proj.path + proj.rpc_files[i]);
		rpc_path = rpc_path.replace(/\\/g, "/");
		rpc_path = path.normalize(rpc_path);
		var rpc_gen = rpc_path.substring(0, rpc_path.lastIndexOf('.'));

		var deps = [rpc_path, rpc_exe];
		var outs = [rpc_gen + '.c.cpp', rpc_gen + '.c.h', rpc_gen + '.s.cpp', rpc_gen + '.s.h'];
		if (this.needUpdate(deps, outs)) {
			deleteFiles(outs);
		    this.writeLog('GenRPC', rpc_path)
		    var cmdline = '{0} {1}'.format(rpc_exe, rpc_path);
			this.addTask(cmdline, outs, deps);
		}
	}
}

AppBuilder.prototype.buildDDL = function (proj) {
	if(proj.ddl_files.length==0) return;

	this.build('DDLGen');

	var ddl_exe = path.normalize(this.output_dir + 'DDLGen' + this.exe_ext);
	for(var i=0; i<proj.ddl_files.length; i++) {
		var ddl_path = proj.path + proj.ddl_files[i];
		ddl_path = ddl_path.replace(/\\/g, "/");
		ddl_path = path.normalize(ddl_path);
		var ddl_gen = ddl_path.substring(0, ddl_path.lastIndexOf('.'));

		var deps = [ddl_path, ddl_exe];
		var outs = [ddl_gen + '.cpp', ddl_gen + '.h'];
		if (this.needUpdate(deps, outs)) {
			deleteFiles(outs);
			this.writeLog('GenDDL', ddl_path)
		    var cmdline = '{0} {1}'.format(ddl_exe, ddl_path);
			this.addTask(cmdline, outs, deps);
		}
	}
}

AppBuilder.prototype.buildBIN = function (proj) {
	var inc_cmd = '';
	for(var i=0; i<proj.inc_dir.length; i++) {
		if(i>0) inc_cmd += ' ';
		var inc_path = process.cwd() + '\\' + proj.inc_dir[i];
		inc_path = inc_path.replace(/\\/g, "/");
		inc_path = path.normalize(inc_path);
		inc_cmd += '-I' + inc_path;
	}

	var obj_path = this.object_dir + proj.name + '/';
	mkdirp_sync(path.normalize(obj_path));
	proj.objs = [];
	for(var i=0; i<proj.src_files.length; i++) {
		var src_path = proj.path + proj.src_files[i];
		src_path = src_path.replace(/\\/g, "/");
		var dst_path = proj.src_files[i];
		dst_path = dst_path.replace(/\\/g, "/");
		dst_path = dst_path.replace(/\//g, "_");
		dst_path = dst_path.replace(/\./g, "_");
		dst_path = obj_path + dst_path;
		src_path = path.normalize(src_path);
		dst_path = path.normalize(dst_path);

		proj.objs.push(dst_path+this.obj_ext);

		var deps = this.loadCPPDepend(dst_path+'.d');
		var outs = [dst_path+this.obj_ext];
		if(!deps) deps = [src_path];
		if(!this.needUpdate(deps, outs)) continue;
		deleteFiles(outs);

		this.writeLog('compile', proj.src_files[i], '[', this.platform, this.config, ']');
		var cpp_flag = ' ';
		var cc_exe = this.cc_exe;
		var file_ext = getFileExt(src_path);
		if(file_ext=='cpp' || file_ext=='cxx' || file_ext=='cc') {
			if(this.cc_exe.substring(0,3)!='cl ') {
				cpp_flag += ' -std=c++11 -stdlib=libc++ ';// -fpermissive '
			}
		} else {
			cc_exe = cc_exe.replace(/clang\+\+/, "clang");
			cc_exe = cc_exe.replace(/g\+\+/, "gcc");
		}

		//console.log(this.cd_exe.format(src_path, inc_cmd + cpp_flag + '-M ' + this.getCCFlags(proj), dst_path+'.d'));
		var cmdline = cc_exe.format(src_path, inc_cmd + cpp_flag + this.getCCFlags(proj), dst_path+this.obj_ext);
		this.addTask(cmdline, outs, deps);
	}

	proj.objs_str = proj.objs.join(' ');
	if(proj.type=='StaticLibrary') {
		var lib_path;
		if(this.platform=='win32') {
			lib_path = this.object_dir + proj.name;
		} else {
			lib_path = this.object_dir + 'lib' + proj.name;
		}
		lib_path = path.normalize(lib_path.replace(/\\/g, "/"));

		var outs = [lib_path + this.lib_ext];
		if (this.needUpdate(proj.objs, outs)) {
		    deleteFiles(outs);
			var cmdline = this.sl_exe.format(this.object_dir, proj.name + this.lib_ext, proj.objs_str);
			this.writeLog('link library', proj.name, '[', this.platform, this.config, ']');
			this.addTask(cmdline, outs, proj.objs);
			this.writeLog();
		}
	} else {
		var exe_path = this.output_dir + proj.name;
		exe_path = path.normalize(exe_path.replace(/\\/g, "/"));
		mkdirp_sync(path.normalize(this.output_dir));

		var dep_files = [];
		var dep_libs = "";
		for(var i=0; i<proj.deps.length; i++) {
			if(this.platform!='win32') {
				dep_files.push(this.object_dir + 'lib' + proj.deps[i] + '.a');
			} else {
				dep_files.push(this.object_dir + proj.deps[i] + '.lib');
			}
			if(dep_libs=="") {
				dep_libs = "-L" + this.object_dir;
			}
			dep_libs += " -l" + proj.deps[i];
		}
		dep_libs += this.getLNFlags(proj);

		var deps = dep_files.concat(proj.objs);
		var outs = [exe_path + this.exe_ext];
		if (this.needUpdate(dep_files.concat(proj.objs), exe_path + this.exe_ext)) {
		    deleteFiles(outs);
			var cmdline = this.ld_exe.format(proj.objs_str, exe_path + this.exe_ext, dep_libs);
			this.writeLog('link execute', proj.name, '[', this.platform, this.config, ']');
			this.addTask(cmdline, outs, deps);
			this.writeLog();
		}
	}
}

AppBuilder.prototype.checkDeps = function (proj) {
	for(var i=0; i<proj.ext_deps.length; i++) {
		if(!this.ext_deps.hasOwnProperty(proj.ext_deps[i])) {
			return false;
		}
	}
	return true;
}

AppBuilder.prototype.buildEXT = function (project_name) {
	if(this.jobs.indexOf(project_name)>=0) return;
	this.jobs.push(project_name);

	var proj = this.solution.getProject(project_name);
	if(!proj) {
		console.log('project '+project_name+' not found! in buildEXT');
		process.exit();
	}


	for(var i=0; i<proj.deps.length; i++) {
		this.buildEXT(proj.deps[i]);
	}

	this.buildRPC(proj);
	this.buildDDL(proj);
}

AppBuilder.prototype.build = function (project_name) {
	if(this.jobs.indexOf(project_name)>=0) return;
	this.jobs.push(project_name);

	var proj = this.solution.getProject(project_name);
	if(!proj) {
		console.log('project, project_name, not found! in build');
		process.exit();
	}

	if(!this.checkDeps(proj)) {
		return;
	}

	for(var i=0; i<proj.deps.length; i++) {
		this.build(proj.deps[i]);
	}

	this.buildBIN(proj);
}

AppBuilder.prototype.clean = function (project_name) {
	if(this.jobs.indexOf(project_name)>=0) return;
	this.jobs.push(project_name);

	var proj = this.solution.getProject(project_name);
	if(!proj) {
		console.log('project '+project_name+' not found! in clean');
		process.exit();
	}

	for(var i=0; i<proj.deps.length; i++) {
		this.clean(proj.deps[i]);
	}

	for(var i=0; i<proj.rpc_files.length; i++) {
		var rpc_path = proj.path + proj.rpc_files[i];
		rpc_path = rpc_path.replace(/\\/g, "/");
		rpc_path = path.normalize(rpc_path);
		var rpc_gen = rpc_path.substring(0, rpc_path.lastIndexOf('.'));

		deleteFile(rpc_gen+'.c.cpp');
		deleteFile(rpc_gen+'.s.cpp');
		deleteFile(rpc_gen+'.c.h');
		deleteFile(rpc_gen+'.s.h');
	}

	for(var i=0; i<proj.ddl_files.length; i++) {
		var ddl_path = proj.path + proj.ddl_files[i];
		ddl_path = ddl_path.replace(/\\/g, "/");
		ddl_path = path.normalize(ddl_path);
		var ddl_gen = ddl_path.substring(0, ddl_path.lastIndexOf('.'));
		deleteFile(ddl_gen+'.h');
		deleteFile(ddl_gen+'.cpp');
	}

	for(var i=0; i<proj.src_files.length; i++) {
		var dst_path = proj.src_files[i];
		dst_path = dst_path.replace(/\\/g, "/");
		dst_path = dst_path.replace(/\//g, "_");
		dst_path = dst_path.replace(/\./g, "_");
		dst_path = this.object_dir + proj.name + '/' + dst_path;
		dst_path = path.normalize(dst_path);
		deleteFile(dst_path+this.obj_ext);
	}

	if(proj.type=='StaticLibrary') {
		var lib_path = this.object_dir + proj.name;
		lib_path = lib_path.replace(/\\/g, "/");
		deleteFile(lib_path+this.lib_ext);
	} else {
		var exe_path = this.output_dir + proj.name;
		exe_path = exe_path.replace(/\\/g, "/");
		deleteFile(exe_path+this.exe_ext);
	}
}

var solution = new SolutionFile();
var tasks = [];
var config = "debug";
var platform = os.platform();
var is_clean = false;
var is_build = false;

function ParseArgs() {
	for(var i=4; i<process.argv.length; i++) {
		var arg_item = process.argv[i];
		var pos = arg_item.indexOf('=');
		if(pos<0) {
			if(arg_item=='all') {
				for(var p=0; p<solution.projs.length; p++) {
					if(tasks.indexOf(solution.projs[p])<0) {
						tasks.push(solution.projs[p].name);
					}
				}
			} else {
				if(!solution.getProject(arg_item)) {
					console.log('project', arg_item, 'not found');
				} else {
					if(tasks.indexOf(arg_item)<0) {
						tasks.push(arg_item);
					}
				}
			}
		} else {
			var name = arg_item.substring(0, pos);
			var value = arg_item.substring(pos+1);
			switch(name) {
			case 'config':
				config = value;
				break;
			case 'platform':
				platform = value;
				break;
			default:
				return false;
			}
		}
	}

	switch(process.argv[3]) {
	case 'build':
		is_build = true;
		break;
	case 'clean':
		is_clean = true;
		break;
	case 'rebuild':
		is_build = true;
		is_clean = true;
		break;
	default:
		return false;
	}

	return true;
}

// start main

if(process.argv.length<4) {
	console.log('invalid parameter');
	process.exit(-1);
}

if(!solution.load(process.argv[2])) {
	console.log('failed to load solution file -', process.argv[2]);
	process.exit(-1);
}

if(!ParseArgs()) {
	console.log('invalid parameter.');
	process.exit(-1);
}

var builder = new AppBuilder(solution, new TaskManager(os.cpus().length));
builder.setBuild(platform, config);

if(is_clean) {
	for(var i=0; i<tasks.length; i++) {
		builder.clean(tasks[i]);
	}
}

if(is_build) {
	for(var i=0; i<tasks.length; i++) {
		builder.buildEXT(tasks[i]);
	}

	builder.manager.build();
	builder.manager.start(function (error) {
		if(error) {
			console.log('build ddl & rpc error!');
            process.exit(-1);
			return;
		}

		var jobs = builder.jobs;
		builder.jobs = [];
		if(jobs.indexOf('DDLGen')>=0) builder.jobs.push('DDLGen');
		if(jobs.indexOf('RpcGen')>=0) builder.jobs.push('RpcGen');
		builder.manager.reset();

		for(var i=0; i<tasks.length; i++) {
			builder.build(tasks[i]);
		}

		builder.manager.build();
		builder.manager.start(function (error) {
			if(error) {
				console.log('build error');
                process.exit(-1);
				return;
			}

			console.log('done');
            process.exit(0);
		});
	});
}
