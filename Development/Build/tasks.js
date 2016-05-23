
var spawn = require('child_process').spawn;

function execute(exe_file, args, callback) {
	var stdout_buffer = "";
	var stderr_buffer = "";
	var exe_proc;
	try {
		exe_proc = spawn(exe_file, args);
	} catch(e) {
		console.log('failed to spawn process.');
		console.log('  ' + e.message);
		console.log('  ' + exe_file, args);//.join(' '));
		callback(-1);
		return;
	}
	var output = [];
	output.push([exe_proc.pid, ":", 'process spwned :', exe_file, args.join(' ')].join(" "));
	exe_proc.stdout.on('data', function (data) {
		stdout_buffer += data;
		for(;;) {
			var pos = stdout_buffer.indexOf('\n');
			if(pos<=0) break;
			output.push([exe_proc.pid, ":", stdout_buffer.substring(0, pos)].join(" "));
			stdout_buffer = stdout_buffer.substring(pos+1);
		}
	});
	exe_proc.stderr.on('data', function (data) {
		stderr_buffer += data;
		for(;;) {
			var pos = stderr_buffer.indexOf('\n');
			if(pos<=0) break;
			output.push([exe_proc.pid, ':', stderr_buffer.substring(0, pos)].join(" "));
			stderr_buffer = stderr_buffer.substring(pos+1);
		}
	});
	exe_proc.on('close', function (code) {
		if(stdout_buffer!="") output.push(stdout_buffer);
		if(stderr_buffer!="") output.push(stderr_buffer);
		output.push([exe_proc.pid, ":", 'process exited, code =', code].join(" "));
		for(var i=0; i<output.length; i++) {
			console.log(output[i]);
		}
		callback(code);
	});
}

function fake_execute(exe_file, args, callback) {
	console.log('execute', exe_file, args);
	callback(0);
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

function TaskManager(slot_max, fake_execute) {
	this.tasks = [];
	this.ready_tasks = [];
	this.files = {};
	this.slot_max = slot_max;
	this.slot_used = 0;
	this.callback = undefined;
	this.has_error = false;
	this.fake_execute = (fake_execute?true:false);
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
	var local_execute = this.fake_execute?fake_execute:execute;
	local_execute(task.task_exe, task.task_args, function(code) {
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

exports.TaskManager = TaskManager;
