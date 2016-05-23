var path = require('path');
var fs = require('fs');

if(!String.prototype.trim) {
	String.prototype.trim = function () {
		return this.replace(/^\s+|\s+$/g, '');
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

function AppBuilder (solution, manager, toolkit) {
	this.solution = solution;
	this.manager = manager;
	this.toolkit = toolkit;

	this.files_data = {};
	this.jobs = [];
	this.verbose = false;

	switch(toolkit.config) {
	case "debug":
		this.output_dir = "../../Binaries/Debug/";
		this.object_dir = "../Intermediate/Debug/";	
		break;
	case "release":
		this.output_dir = "../../Binaries/Release/";
		this.object_dir = "../Intermediate/Release/";	
		break;
	default:
		console.log("invalid config type,", toolkit.config);
		process.exit(-1);
	}
}

AppBuilder.prototype.log = function () {
	if(this.verbose) {
		var args = [];
		for(var i=0; i<arguments.length; i++) {
			args.push(arguments[i]);
		}
		console.log(args.join(' '));
	}
};

AppBuilder.prototype.needUpdate = function (in_files, out_files) {
	if(typeof(in_files)=='string') {
		in_files = [in_files];
	}

	var src_ts = 0;
	var src_filename = "";
	for(var i=0; i<in_files.length; i++) {
		if(!fs.existsSync(in_files[i])) {
			this.log('needUpdate : source file not existed', in_files[i]);
			return true;
		}
		var f_ts = fs.statSync(in_files[i]);
		var ts = Date.parse(f_ts.mtime) + f_ts.mtime.getMilliseconds();
		if(ts>src_ts) {
			src_filename = in_files[i];
			src_ts = ts;
		}
	}

	if(typeof(out_files)=='string') {
		out_files = [out_files];
	}

	for(var i=0; i<out_files.length; i++) {
		if(!fs.existsSync(out_files[i])) {
			this.log('needUpdate : not existd ', src_ts, out_files[i]);
			return true;
		}
		var f_ts = fs.statSync(out_files[i]);
		var ts = Date.parse(f_ts.mtime) + f_ts.mtime.getMilliseconds();
		if(ts < src_ts) {
			this.log('needUpdate : dirty #', new Date(ts), new Date(src_ts), out_files[i], src_filename);
			return true;
		}
	}

	return false;
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
		this.log('loadCPPDepend', 'failed.', filename);
		return undefined;
	}
}

AppBuilder.prototype.buildRPC = function (proj) {
	if(proj.rpc_files.length==0) return;

	this.build('RpcGen');

	var rpc_exe = this.toolkit.genExecuteFile(this.output_dir, "RpcGen");
	for(var i=0; i<proj.rpc_files.length; i++) {
		var rpc_path = path.normalize(proj.path + proj.rpc_files[i]);
		rpc_path = rpc_path.replace(/\\/g, "/");
		rpc_path = path.normalize(rpc_path);
		var rpc_gen = rpc_path.substring(0, rpc_path.lastIndexOf('.'));

		var deps = [rpc_path, rpc_exe];
		var outs = [rpc_gen + '.c.cpp', rpc_gen + '.c.h', rpc_gen + '.s.cpp', rpc_gen + '.s.h'];
		if (this.needUpdate(deps, outs)) {
			deleteFiles(outs);
		    this.log('GenRPC', rpc_path)
		    this.log(rpc_exe, rpc_path);
			this.manager.addTask(rpc_exe, [rpc_path], outs, deps);
		}
	}
}

AppBuilder.prototype.buildDDL = function (proj) {
	if(proj.ddl_files.length==0) return;

	this.build('DDLGen');

	var ddl_exe = this.toolkit.genExecuteFile(this.output_dir, 'DDLGen');
	for(var i=0; i<proj.ddl_files.length; i++) {
		var ddl_path = proj.path + proj.ddl_files[i];
		ddl_path = ddl_path.replace(/\\/g, "/");
		ddl_path = path.normalize(ddl_path);
		var ddl_gen = ddl_path.substring(0, ddl_path.lastIndexOf('.'));

		var deps = [ddl_path, ddl_exe];
		var outs = [ddl_gen + '.cpp', ddl_gen + '.h'];
		if (this.needUpdate(deps, outs)) {
			deleteFiles(outs);
			this.log('GenDDL', ddl_path)
			this.log(ddl_exe, ddl_path);
			this.manager.addTask(ddl_exe, [ddl_path], outs, deps);
		}
	}
}

AppBuilder.prototype.buildBIN = function (proj) {
	var incs = [].concat(proj.inc_dir);//.concat(this.toolkit.getOption('incs', proj.ext_deps));
	var defs = []; //.concat(this.toolkit.getOptions(defs, proj.deps));

	var obj_path = this.object_dir + proj.name + '/';
	mkdirp_sync(path.normalize(obj_path));
	proj.objs = [];
	for(var i=0; i<proj.src_files.length; i++) {
		if(!this.toolkit.isSupport(proj.src_files[i])) {
			continue;
		}

		var src_path = proj.path + proj.src_files[i];
		src_path = src_path.replace(/\\/g, "/");
		var dst_path = this.toolkit.genObjectFile(obj_path, proj.src_files[i]);
		src_path = path.normalize(src_path);
		dst_path = path.normalize(dst_path);

		proj.objs.push(dst_path);

		var deps = this.loadCPPDepend(dst_path+'.d');
		if(!deps) deps = [src_path];
		deps.push(dst_path+'.d');
		var outs = [dst_path];
		if(!this.needUpdate(deps, outs)) continue;
		deleteFiles(outs);
		deleteFile(dst_path+'.d');

		var cmd = this.toolkit.compileDefine(dst_path+'.d', src_path, incs, defs, proj.ext_deps);
		this.manager.addTask(cmd.cmd, cmd.args, [dst_path+'.d'], [src_path]);
		this.log(cmd.cmd, cmd.args.join(" "));
		cmd = this.toolkit.compile(dst_path, src_path, incs, defs, proj.ext_deps);
		this.manager.addTask(cmd.cmd, cmd.args, outs, deps);
		this.log(cmd.cmd, cmd.args.join(" "));
	}

	proj.objs_str = proj.objs.join(' ');
	if(proj.type=='StaticLibrary') {
		var lib_path = this.toolkit.genStaticLibrary(this.object_dir, proj.name);
		lib_path = path.normalize(lib_path.replace(/\\/g, "/"));

		var outs = [lib_path];
		if (this.needUpdate(proj.objs, outs)) {
		    deleteFiles(outs);
			var cmd = this.toolkit.buildStaticLibrary(lib_path, proj.objs, proj.ext_deps);
			this.log(cmd.cmd, cmd.args.join(" "));
			this.manager.addTask(cmd.cmd, cmd.args, outs, proj.objs);
		}
	} else {
		var exe_path = this.toolkit.genExecuteFile(this.output_dir, proj.name);
		exe_path = path.normalize(exe_path.replace(/\\/g, "/"));
		mkdirp_sync(path.normalize(this.output_dir));

		var libs = [].concat(proj.deps);//.concat(this.toolkit.getOption('libs', proj.deps));
		var lib_dirs = [this.object_dir].concat(this.toolkit.getOption('libs_dir', proj.ext_deps));

		var deps = [].concat(proj.objs);
		for(var i=0; i<proj.deps.length; i++) {
			var lib_path = this.toolkit.genStaticLibrary(this.object_dir, proj.deps[i]);
			lib_path = path.normalize(lib_path.replace(/\\/g, "/"));
			deps.push(lib_path);
		}
		var outs = [exe_path];

		if (this.needUpdate(deps, outs)) {
		    deleteFiles(outs);
			var cmd = this.toolkit.buildProgram(exe_path, proj.objs, libs, lib_dirs, proj.ext_deps);
			this.log(cmd.cmd, cmd.args.join(" "));
			this.manager.addTask(cmd.cmd, cmd.args, outs, deps);
		}
	}
}

AppBuilder.prototype.checkExtDeps = function (proj) {
	return this.toolkit.checkExtDeps(proj.ext_deps);
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

	if(!this.checkExtDeps(proj)) {
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

	var obj_path = this.object_dir + proj.name + '/';
	for(var i=0; i<proj.src_files.length; i++) {
		var dst_path = this.toolkit.genObjectFile(obj_path, proj.src_files[i]);
		dst_path = path.normalize(dst_path);
		deleteFile(dst_path);
		deleteFile(dst_path + '.d');
	}

	if(proj.type=='StaticLibrary') {
		var lib_path = this.toolkit.genStaticLibrary(this.object_dir, proj.name);
		lib_path = path.normalize(lib_path.replace(/\\/g, "/"));
		deleteFile(lib_path);
	} else {
		var exe_path = this.toolkit.genExecuteFile(this.output_dir, proj.name);
		exe_path = path.normalize(exe_path.replace(/\\/g, "/"));
		deleteFile(exe_path);
	}
}

exports.AppBuilder = AppBuilder;
