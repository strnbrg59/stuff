package net.trhj.cardation;

import android.util.Log;
import android.widget.Toast;
import java.io.*;
import java.util.*;

public class CardationUtils
{
    public static int epochNow() {
        Date now = new Date();
        long now_secs = now.getTime();
        return (int)(now_secs/1000);
    }

    public static void logStackTrace() {
        for (StackTraceElement ste : Thread.currentThread().getStackTrace()) {
            Log.w("Cardation", ste.toString());
        }
    }

    // Checks for a duplicate.
    public static Boolean batchContainsRecto(LinkedList<CardDb.Card> batch,
                                             String needle) {
        for (int i=0; i<batch.size(); ++i) {
            if (batch.get(i).recto_.equals(needle)) {
                return true;
            }
        }
        return false;
    }

    /* We pass rand_reversal_prob, instead of just use G.rand_reversal, cuz
     * sometimes we want to get just the "deterministic" result.
     */
    public static boolean shouldGoForward(Boolean active,
                                          int fwd_streak,
                                          double rand_reversal_prob)
    {
        // Doesn't depend on bkwd_streak, doesn't need to.
        boolean forward = (!active) || (fwd_streak < 0);
        if (!forward) {
            Random r = new Random(epochNow());
            int rr = r.nextInt(101);
            if (rr < 100*rand_reversal_prob) {
                forward = true;
            }
        }
        return forward;
    }

    public static int dueDate(int fwd_streak, int bkwd_streak, Boolean active)
    {
        int streak;
        boolean new_forward = shouldGoForward(active, fwd_streak, 0.0);
        if (new_forward == true) {
            streak = fwd_streak;
        } else {
            streak = bkwd_streak;
        }

        // The delay is 0 if streak<0, otherwise it's one day times 2^streak.
        // We add some randomness too, to ensure that a whole bunch of cards
        // don't all come due at the same time.
        final int day_secs = 24*3600;
        int now = epochNow();
        Random r = new Random(now);
        int delay;
        if (streak < 0) {
            delay = 0;
        } else {
            // XXX As we approach 2036, everything will bunch up against the
            // maximal Unix epoch.
            double ideal_delay = day_secs *
                Math.pow(2, streak) * (1.0 + (r.nextInt(101) - 50)/1000.0);
            int max_int = (int)(Math.pow(2, 31) - 1);
            delay = (int)
                Math.min(ideal_delay, max_int - now);
        }

        return now + delay;
    }

    static List<List<String>> dbbk2records(InputStream instream)
    {
        StringBuilder all_str = new StringBuilder();
        final int toks_per_line = 7;
        // XXX Compute that from some description of the DB schema.
        List<List<String>> result = null;

        try {
            BufferedReader reader =
                new BufferedReader(new InputStreamReader(instream, "UTF-8"),
                                   2048);

            // The dbbk file has newlines only where I deliberately introduced
            // them, and at the end of a word's entire record (i.e. after the
            // quote).  If a long line (typically in the quote) looks broken on
            // the device, those aren't newlines and they don't appear in the
            // backup file.
            String theline = reader.readLine();
            while (theline != null)
            {
                Log.d("Cardation", "theline = |" + theline + "|");
                all_str.append(theline);
                all_str.append('\n');
                theline = reader.readLine();
            }
            if (all_str.toString().isEmpty()) {
                return result;
            }
            all_str.setLength(all_str.length()-1);
        }
        catch(IOException e) {
            Log.e("Cardation", "IOException 1");
            return null;
        }

        String[] all_toks = all_str.toString().split("@");
        if (all_toks.length < toks_per_line)
        {
            Log.e("Cardation", "all_toks.length = " + all_toks.length +
                  " (< toks_per_line)");
            return null;
        }

        Log.w("Cardation", "all_str has been split, all_toks.length = "
                + all_toks.length);
        result = new ArrayList<List<String>>();
        int i = 0;
        while (i < all_toks.length)
        {
            List<String> record = new ArrayList<String>();
            for (int j=0;j<toks_per_line;++j)
            {
                String tok = all_toks[i];
                ++i;

                if ((tok!=null) && (tok.length()>0) && (tok.charAt(0) == '\n')){
                    Log.i("Cardation", "|" + tok + "|.charAt(0) is newline");
                    tok = tok.substring(1);
                }

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
