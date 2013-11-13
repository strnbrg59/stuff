package net.trhj.androidflashcards;

import java.util.*;
import android.util.Log;
import android.content.Intent;
import android.content.Context;
import android.widget.Toast;
import java.io.InputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.util.Random;

public class CardationUtils
{
    public static int epochNow() {
        Date now = new Date();
        long now_secs = now.getTime();
        return (int)(now_secs/1000);
    }

    public static void sendEmail(String addressee, String subject,
                                 String content,
                                 Context context) {
        Intent i = new Intent(Intent.ACTION_SEND);
        i.setType("message/rfc822");
        i.putExtra(Intent.EXTRA_EMAIL,
                   new String[]{addressee});
        i.putExtra(Intent.EXTRA_SUBJECT, subject);
        i.putExtra(Intent.EXTRA_TEXT, content);
        try {
            context.startActivity(Intent.createChooser(i, "Send mail..."));
        } catch (android.content.ActivityNotFoundException ex) {
            Toast.makeText(context,
            "There are no email clients installed.", Toast.LENGTH_LONG).show();
        }
    }

    public static void logStackTrace() {
        for (StackTraceElement ste : Thread.currentThread().getStackTrace()) {
            Log.w("Cardation", ste.toString());
        }
    }

    static List<List<String>> dbbk2records(InputStream instream)
    {
        StringBuilder all_str = new StringBuilder();
        final int toks_per_line = 7;
        // XXX Compute that from some description of the DB schema.

        try {
            BufferedReader reader =
                new BufferedReader(new InputStreamReader(instream, "UTF-8"),
                                   2048);

            String theline = reader.readLine();
            while (theline != null)
            {
                Log.d("Cardation", "theline = " + theline);
                all_str.append(theline);
                theline = reader.readLine();
            }
        }
        catch(IOException e) {
            Log.e("Cardation", "IOException 1");
            return null;
        }

        List<List<String>> result = new ArrayList<List<String>>();
        String[] all_toks = all_str.toString().split("@");
        if (all_toks.length < toks_per_line)
        {
            Log.e("Cardation", "all_toks.length = " + all_toks.length +
                  " (< toks_per_line)");
            return null;
        }

        Log.w("Cardation", "all_str has been split, all_toks.length = "
                + all_toks.length);
        int i = 0;
        while (i < all_toks.length)
        {
            List<String> record = new ArrayList<String>();
            for (int j=0;j<toks_per_line;++j)
            {
                String tok = all_toks[i];
                ++i;

                // Throw away defective lines (only "quote" field can be empty).
                if (tok.equals("") && (j != toks_per_line-1))
                {
                    Log.e("Cardation", "tok = " + tok + ", i = " + i +
                            ", j = " + j + ", bailing out");
                    return null;
                }

                record.add(tok);
                if (i > all_toks.length - 1)
                {
                    // Quirk when last line ends in "@@" (i.e. no quote).
                    // Doesn't matter if earlier lines end in "@@".
                    record.add("");
                    break;
                }
            }
            result.add(record);
        }

        return result;
    }

    /* Useful for returning early without having to comment out lots of stuff
     * (as javac refuses to compile code that can't be reached).
     */
    public static boolean almostSurely()
    {
        Random r = new Random();
        return (r.nextInt(10000) > 0);
    }
}