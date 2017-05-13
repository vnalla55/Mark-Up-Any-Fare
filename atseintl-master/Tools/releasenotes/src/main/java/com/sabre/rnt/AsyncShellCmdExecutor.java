package com.sabre.rnt;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;


public class AsyncShellCmdExecutor {

	private static HashMap<String, AsyncShellCmdExecutor> executors;
	
	private String command;
	private LinkedList<String> stdoutLines = new LinkedList<>(), stderrLines = new LinkedList<>();
	volatile private Integer exitStatus = null; // If finished, will be different than null
	
	static AsyncShellCmdExecutor getExecutorForCmd(String cmd) {
		if (executors.containsKey(cmd)) {
			return executors.get(cmd);
		}
		
		AsyncShellCmdExecutor newExecutor = new AsyncShellCmdExecutor(cmd);
		executors.put(cmd, newExecutor);
		return newExecutor;
	}
	
	private class CommandRunner extends Thread {
		@Override
		public void run() {
			try {
				String line;
				Process p = Runtime.getRuntime().exec(
						new String[] { "sh", "-c", command });
				BufferedReader bri = new BufferedReader(new InputStreamReader(
						p.getInputStream()));
				BufferedReader bre = new BufferedReader(new InputStreamReader(
						p.getErrorStream()));
				while ((line = bri.readLine()) != null) {
					synchronized (AsyncShellCmdExecutor.this) {
						stdoutLines.add(line);
					}
				}
				bri.close();
				while ((line = bre.readLine()) != null) {
					synchronized (AsyncShellCmdExecutor.this) {
						stderrLines.add(line);
					}
				}
				bre.close();
				exitStatus = p.waitFor();
			} catch (Exception err) {
				err.printStackTrace();
			}
		}
	}
	
	private AsyncShellCmdExecutor(String command) {
		this.command = command;
		new CommandRunner().start();
	}
	
	public synchronized List<String> getNewStdoutLines() {
		List<String> result = stdoutLines;
		stdoutLines = new LinkedList<>();
		return result;
	}
	
	public synchronized List<String> getNewStderrLines() {
		List<String> result = stderrLines;
		stderrLines = new LinkedList<>();
		return result;
	}
	
	public boolean finished() {
		return exitStatus != null;
	}
	
	public Integer exitStatus() {
		return exitStatus;
	}
}
