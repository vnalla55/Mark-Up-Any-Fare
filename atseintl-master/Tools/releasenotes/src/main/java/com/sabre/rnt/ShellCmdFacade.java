package com.sabre.rnt;

import java.io.BufferedReader;
import java.io.InputStreamReader;

public class ShellCmdFacade {

    interface Logger {
        void addLine(String line);
    }

    private String cmd;
    private Logger logger;

    ShellCmdFacade(String cmd, Logger logger) {
        this.cmd = cmd;
        this.logger = logger;
    }

    int run(boolean quiet) {
        int result = 1;
        try {
            if (! quiet) {
                logger.addLine("Running command '" + cmd + "'...");
            }

            ProcessBuilder pb =
                    new ProcessBuilder("sh", "-c", cmd);

            pb.redirectErrorStream(true);
            Process p = pb.start();

            String line;
            BufferedReader bri = new BufferedReader(new InputStreamReader(
                    p.getInputStream()));
            while ((line = bri.readLine()) != null) {
                  logger.addLine(line);
            }
            bri.close();
            result = p.waitFor();
        } catch (Exception err) {
            err.printStackTrace();
        } finally {
            if (result != 0) {
                  logger.addLine("Running '" + cmd + "' has failed.");
            }
            return result;
        }
    }

}
