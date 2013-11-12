package net.trhj.androidflashcards;

import java.io.*;
import android.os.Environment;

class BackupVersion  {

    public static int newestBackupVersion(final String language)
    {
        String[] dirlist = listDbbk(language);

        int result = -1;
        for (int i=0; i<dirlist.length; ++i) {
            String[] parts1 = dirlist[i].split("\\.");
            String base = parts1[0];
            String[] parts2 = base.split("_");
            String str_suffix = parts2[4];
            int int_suffix = Integer.parseInt(str_suffix);
            if (int_suffix > result) {
                result = int_suffix;
            }
        }
        return result;
    }

    /* The one with the highest numerical suffix. */
    public static String newestBackupFile(final String language)
    {
        int highest_version = newestBackupVersion(language);
        if (highest_version == -1) {
            return "";
        } else {
            return Environment.getExternalStorageDirectory() + "/" 
                + CardDb.pkg_underbars_ + "_" + language + "_"
                + highest_version
                + ".dbbk";
        }
    }

    public static String nextBackupFile(final String language)
    {
        int highest_version = newestBackupVersion(language);
        return Environment.getExternalStorageDirectory() + "/"
            + CardDb.pkg_underbars_ + "_" + language + "_"
            + (highest_version+1)
            + ".dbbk";
    }        

    public static String[] listDbbk(final String language)
    {
        File dir = new File(Environment.getExternalStorageDirectory() + "/");
        String[] dirlist = dir.list(
            new FilenameFilter() {
                public boolean accept(File dir, String name) {
                    // E.g. net_trhj_androidflashcards_Italian_2.dbbk
                    String[] parts = name.split("\\.");
                    if (parts.length != 2) {
                        return false;
                    } else {
                        String base = parts[0];
                        String extension = parts[1];
                        if (!extension.equals("dbbk")) {
                            return false;
                        } else {
                            String[] base_tokens = base.split("_");
                            if (base_tokens.length != 5) {
                                return false;
                            } else {
                                if (!base_tokens[3].equals(language)) {
                                    return false;
                                }
                            }
                        }
                    }
                    return true;
                }
            });
        
        String[] result = new String[dirlist.length];
        for (int i=0;i<dirlist.length;++i) {
            result[i] = dirlist[i];
        }
        return result;
    }
}
