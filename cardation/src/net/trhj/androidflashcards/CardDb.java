/* Based on LoaderThrottleSupport.java and the online database tutorial */

package net.trhj.androidflashcards;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.Cursor;
import android.widget.Toast;
import android.content.ContentValues;
import android.provider.BaseColumns;
import android.util.Log;
import android.os.Environment;
import java.lang.Math;
import java.util.*;
import java.io.*;

public class CardDb {

    private static final int database_version_ = 8;
    public static String pkg_underbars_ = "net_trhj_androidflashcards";
    public static String curr_language_ = null;
    static int n_due_ = 0;
    static int cram_increment_ = 0;

    static public int getCramIncrement()
    {
        return cram_increment_;
    }
    static public void setCramIncrement(int cram_widget_val)
    {
        // pow(2,cram_widget_value) days, or zero, bounded by 1970+2^31
        if (cram_widget_val == 0) {
            cram_increment_ = 0;
        } else {
            final double day_secs = 24*3600;
            final int now = CardationUtils.epochNow();
            double ideal_cram = Math.pow(2, cram_widget_val) * day_secs;
            double max_cram = (int)(Math.pow(2, 31) - 1) - now;
            cram_increment_ = (int) Math.min(max_cram, ideal_cram);
            Log.w("Cardation", "cram_increment_ = " + cram_increment_/86400.
                    + " days");
        }
    }

    static public int getNDue()
    {
        return n_due_;
    }

    static public void setCurrLanguage(String lang, Context context)
    {
        Log.w("Cardation", "curr_language_ set to " + lang);
        curr_language_ = lang;
    }

    static public String getCurrLanguage()
    {
        return curr_language_;
    }

    static String getTableName()
    {
        if (getCurrLanguage() == null) {
            Log.e("Cardation", "curr_language_ = null !?!");
        }
        return pkg_underbars_ + "_" + getCurrLanguage();
    }

    /* Layout of language tables */
    public static final class Contract implements BaseColumns {
        private Contract() {}
    
        public static final String RECTO = "recto";
        public static final String VERSO = "verso";
        public static final String FWD_STREAK = "fwd_streak";
        public static final String BKWD_STREAK = "bkwd_streak";
        public static final String DUE = "due";
        public static final String IMPORTANCE = "importance";
        public static final String QUOTE = "quote";

        public static final int recto = 0;
        public static final int verso = 1;
        public static final int fwd_streak = 2;
        public static final int bkwd_streak = 3;
        public static final int due = 4;
        public static final int importance = 5;
        public static final int quote = 6;
    }

    /**
     * This class helps open, create, and upgrade the database file.
     */
    static class DbHelper extends SQLiteOpenHelper {
    
        private static final String DATABASE_NAME = "cardation"
            + Integer.toString(database_version_) + ".db";
        private static Boolean first_time_ = true;
        Context context_;
    
        DbHelper(Context context) {
            super(context, DATABASE_NAME, null, database_version_);
            context_ = context;
            if (first_time_) {
                // showTableInfo();
                first_time_ = false;
            }
        }
    
        void showTableInfo() {
            SQLiteDatabase db = this.getReadableDatabase();
            Cursor cursor = db.rawQuery("PRAGMA TABLE_INFO("
                + getTableName() + ")", null);
            if (cursor.moveToFirst()) {
                // Each row is a field of the table.
                do {
                    Log.w("Cardation", cursor.getString(0) + " " +
                          cursor.getString(1));
                } while (cursor.moveToNext());
            }
            cursor.close();
            db.close();
        }
    
        @Override
        public SQLiteDatabase getReadableDatabase() {
            SQLiteDatabase db = super.getReadableDatabase();
            createTableIfNotExists(db);
            return db;
        }

        @Override
        public SQLiteDatabase getWritableDatabase() {
            SQLiteDatabase db = super.getWritableDatabase();
            createTableIfNotExists(db);
            return db;
        }

        public void createTableIfNotExists(SQLiteDatabase db) {
            if (getTableName() == "null") {
                Log.w("Cardation",
                      "Current language = null, not creating a table for it.");
                return;
            }
            db.execSQL("CREATE TABLE IF NOT EXISTS " + getTableName() + " ("
                + Contract.RECTO + " TEXT PRIMARY KEY,"
                + Contract.VERSO + " TEXT" + ","
                + Contract.FWD_STREAK + " INTEGER" + ","
                + Contract.BKWD_STREAK + " INTEGER" + ","
                + Contract.DUE + " INTEGER" + ","
                + Contract.IMPORTANCE + " INTEGER" + ","
                + Contract.QUOTE + " TEXT"
                + ");");
        }
    
        @Override
        public void onCreate(SQLiteDatabase db) {
            createTableIfNotExists(db);
        }
    
       /**
        * A real application should upgrade the database in place.
        */
       @Override
       public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion)
       {
           backup(context_);
    
           // Logs that the database is being upgraded
           Log.w("Cardation",
                "Upgrading database from version " + oldVersion + " to "
                   + newVersion + ", which will destroy all old data");
        
           // Kills the table and existing data
           /* Let's not!
           db.execSQL("DROP TABLE IF EXISTS " + getTableName());
           */
    
           // Recreates the database with a new version
           onCreate(db);
       }
    }

    public static long saveCard(Context context, Card card)
    {
        DbHelper helper = new DbHelper(context);
        SQLiteDatabase db = helper.getWritableDatabase();

        ContentValues values = new ContentValues();
        values.put(Contract.RECTO, card.recto_);
        values.put(Contract.VERSO, card.verso_);
        values.put(Contract.FWD_STREAK, card.fwd_streak_);
        values.put(Contract.BKWD_STREAK, card.bkwd_streak_);
        values.put(Contract.DUE, card.due_);
        values.put(Contract.IMPORTANCE, card.importance_);
        values.put(Contract.QUOTE, card.quote_);

        long newRowId =
            db.insert(getTableName(), null, values);

        db.close();
        helper.close();
        return newRowId;
    }

    public final static class Card implements Cloneable {
        public Card(String recto, String verso,
                    int fwd_streak, int bkwd_streak,
                    int due,
                    int importance,
                    String quote) {
            recto_ = recto;
            verso_ = verso;
            fwd_streak_ = fwd_streak;
            bkwd_streak_ = bkwd_streak;
            due_ = due;
            importance_ = importance;
            quote_ = quote;
        }

        public Object clone() {
            try {
                return super.clone();
            }
            catch(Exception e) {
                Log.e("Cardation", "Card::clone() failed");
                return null;
            }
        }

        public String       recto_;
        public String       verso_;
        public int          fwd_streak_; 
        public int          bkwd_streak_;
        public int          due_;
        public int          importance_; 
        public String       quote_;
    }
    

    public static LinkedList<String> listLanguages(Context context) {
        DbHelper helper = new DbHelper(context);
        SQLiteDatabase db = helper.getReadableDatabase();
        Cursor cursor = db.rawQuery(
            "SELECT NAME FROM sqlite_master WHERE TYPE='table'",
            null);
        Log.w("Cardation",
              "Table-list query, #rows = " + cursor.getCount());

        LinkedList<String> result = new LinkedList<String>();
        cursor.moveToFirst();
        while (!cursor.isAfterLast()) {
            String table_name = cursor.getString(0);
            Log.w("Cardation", "table: |" + table_name + "|");
            if (   table_name.equals("android_metadata")
                || table_name.equals(ConfigDb.table_name_)) {
                cursor.moveToNext();
                continue;
            }

            // Language name is last token.
            StringTokenizer toker = new StringTokenizer(table_name, "_");
            int n_tokens = toker.countTokens();
            for (int i=0; i<n_tokens-1; ++i) {
                toker.nextToken();
            }
            String language = toker.nextToken();
            Log.w("Cardation", "language = |" + language + "|");
            if (!language.equals("null")) {
                result.add(language);
            }

            cursor.moveToNext();
        }

        cursor.close();
        db.close();
        return result;
    }


    static Card makeCard(Cursor cursor)
    {
        int recto_column =
            cursor.getColumnIndexOrThrow(Contract.RECTO);
        int verso_column =
            cursor.getColumnIndexOrThrow(Contract.VERSO);
        int fwd_streak_column =
            cursor.getColumnIndexOrThrow(Contract.FWD_STREAK);
        int bkwd_streak_column =
            cursor.getColumnIndexOrThrow(Contract.BKWD_STREAK);
        int due_column =
            cursor.getColumnIndexOrThrow(Contract.DUE);
        int importance_column =
            cursor.getColumnIndexOrThrow(Contract.IMPORTANCE);
        int quote_column =
            cursor.getColumnIndexOrThrow(Contract.QUOTE);

        Card result = new Card(cursor.getString(recto_column),
                    cursor.getString(verso_column),
                    Integer.parseInt(cursor.getString(fwd_streak_column)),
                    Integer.parseInt(cursor.getString(bkwd_streak_column)),
                    Integer.parseInt(cursor.getString(due_column)),
                    Integer.parseInt(cursor.getString(importance_column)),
                    cursor.getString(quote_column));
        return result;
    }

    //
    // Find cards whose due date is earlier than now.  Limit the batch to
    // max_cards, to keep the work load reasonable.
    //
    public static LinkedList<Card> getDueCards(Context context,
                                               int max_cards)
    {
        DbHelper helper = new DbHelper(context);
        SQLiteDatabase db = helper.getReadableDatabase();
        String query = new String("SELECT * FROM " + getTableName()
                + " WHERE " + Contract.DUE + " < " +
                ((int)CardationUtils.epochNow() + 1 + getCramIncrement()));
        Log.w("Cardation", "Query = " + query);

        Cursor cursor = db.rawQuery(query, null);
        // The "ORDER BY" ensures that we'll get the same batch when
        // LearnActivity requeries (which it does when it gets to the end of the
        // batch but wants to bring back any cards whose due date has come up
        // while we were doing the quiz (or more likely, cards in the batch
        // whose streak stands at <0 still).
        // The quote "+ 1" is for when this function is called a 2nd (3rd...)
        // time during the same learning session, which we do so we see a card
        // repeatedly until its streak rises to 0.  Without that "+ 1", we'd
        // likely get to this db.rawQuery within the same second in which we
        // timestamped that last card's due date.  And then we wouldn't pick up
        // that card in this query!

        LinkedList<Card> result = new LinkedList<Card>();
        n_due_ = cursor.getCount();
        if (n_due_ == 0) {
            Log.w("Cardation", "n_due_ = 0 in getDueCards()");
            db.close();
            helper.close();
            return result;
        }

        cursor.moveToFirst();
        do {
            Card newCard = makeCard(cursor);
            result.add(newCard);
            Log.i("Cardation", "getDueCards(): " +
                  newCard.recto_ + ", now-due(string) = "
                + ((int)CardationUtils.epochNow() - newCard.due_));

        } while (cursor.moveToNext() && (result.size() < max_cards));

        int now = CardationUtils.epochNow();
        Collections.shuffle(result, new Random(now));

        Log.w("Cardation", "Retrieved " + result.size() + " due cards.");
        db.close();
        helper.close();
        return result;
    }

    public static Card findCard(Context context, String recto)
    {
        DbHelper helper = new DbHelper(context);
        SQLiteDatabase db = helper.getWritableDatabase();
        Cursor cursor = db.rawQuery("SELECT * from " +
            getTableName() + " WHERE " + Contract.RECTO +
            " = \"" + recto + "\"",
            null);
        cursor.moveToFirst();
        Card result = makeCard(cursor);

        cursor.close();     
        db.close();
        helper.close();

        return result;
    }

    public static void updateByRecto(Context context, Card new_card)
    {
        String selection = Contract.RECTO + " LIKE ?";
        String[] selectionArgs = { String.valueOf(new_card.recto_) };
        update(context, new_card, selection, selectionArgs);
    }

    public static void update(Context context, Card new_card,
                              String selection,
                              String[] selectionArgs) {
        DbHelper helper = new DbHelper(context);
        SQLiteDatabase db = helper.getWritableDatabase();

        ContentValues values = new ContentValues();
        values.put(Contract.RECTO, new_card.recto_);
        values.put(Contract.VERSO, new_card.verso_);
        values.put(Contract.FWD_STREAK, new_card.fwd_streak_);
        values.put(Contract.BKWD_STREAK, new_card.bkwd_streak_);
        values.put(Contract.DUE, new_card.due_);
        values.put(Contract.IMPORTANCE, new_card.importance_);
        values.put(Contract.QUOTE, new_card.quote_);

        double now = CardationUtils.epochNow();
        Log.w("Cardation", "update(): new_card.due_ = now + " +
                (new_card.due_-now)  + " seconds from now");

        int count = db.update(getTableName(),
                              values,
                              selection,
                              selectionArgs);
        Log.w("Cardation", "db.update() returned " + count);

        db.close();
        helper.close();
    }

    public static void deleteCard(Context context, Card card)
    {
        DbHelper helper = new DbHelper(context);
        SQLiteDatabase db = helper.getWritableDatabase();

        String selection = Contract.RECTO + " LIKE ?";
        String[] selectionArgs = { String.valueOf(card.recto_) };
        db.delete(getTableName(), selection, selectionArgs);
        Log.w("Cardation", "Deleted card " + card.recto_);

        db.close();
        helper.close();
    }

    public static boolean isExternalStorageWritable()
    {
        String state = Environment.getExternalStorageState();
        if (Environment.MEDIA_MOUNTED.equals(state)) {
            return true;
        }
        return false;
    }

    public static LinkedList<String> listAllRecto(Context context) {
        LinkedList<String> result = new LinkedList<String>();
        DbHelper helper = new DbHelper(context);
        SQLiteDatabase db = helper.getReadableDatabase();
        Cursor cursor = db.rawQuery( "SELECT * FROM " + getTableName(), null);
        for (int r=0; r<cursor.getCount(); ++r) {
            cursor.moveToPosition(r);
            result.add(cursor.getString(0));
        }

        cursor.close();
        db.close();
        helper.close();

        Collections.sort(result);
        return result;
    }

    public static void backup(Context context) {
        /* Assemble it as one long string, which you will email, as well as
         * write to /mnt/sdcard.
         */
        DbHelper helper = new DbHelper(context);
        SQLiteDatabase db = helper.getReadableDatabase();
        Cursor cursor = db.rawQuery( "SELECT * FROM " + getTableName(), null);
        String[] col_names = cursor.getColumnNames();
        String everything = new String();
        for (int r=0; r<cursor.getCount(); ++r) {
            cursor.moveToPosition(r);
            for (int c=0; c<col_names.length; ++c) {
                String delimiter = "@";
                everything += cursor.getString(c);
                everything += delimiter;
            }
            everything += '\n';
        }
        if (db != null) {
            cursor.close();
            db.close();
            helper.close();
        }
        Log.w("Cardation", "Preparing to backup " + cursor.getCount()
              + " records.");

        /*
         * Send it all out as email, in case /mnt/sdcard isn't writable (as
         * indeed it's not on Joe's Kyocera).
         */
        try {
            CardationUtils.sendEmail("strnbrg59@gmail.com",
                                     getCurrLanguage() + " backup",
                                     everything, context);
        } catch(Exception ex) {
            Log.w("Cardation", "Could not send out email, ok.");
        }

        /*
         * Write a backup file on /mnt/sdcard.
         */
        if (!isExternalStorageWritable()) {
            Log.w("Cardation", "External storage is not writable");
            return;
        }

        // Compiler insists we put this in a try-catch block:
        try {
            File outfile =
                new File(BackupVersion.nextBackupFile(getCurrLanguage()));
            Log.i("Cardation",
                "outfile.getAbsolutePath() = " + outfile.getAbsolutePath());
            Log.i("Cardation", "outfile.canWrite() = " + outfile.canWrite());

            OutputStream outstream = new FileOutputStream(outfile);
            BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(
                outstream, "UTF-8"));
            writer.write(everything);
            writer.close();
            Log.w("Cardation", "Backed up " + outfile.getAbsolutePath());
        } catch (Exception ex) {
            Log.e("Cardation", "Could not write backup to /mnt/sdcard.");
        }
    }

    /* Reads a text representation and updates the DB with it, using recto as
     * the key.  If key is already present, updates the record.  Otherwise,
     * creates an all-new record.
     */
    public static void restore(Context context) {
        if (!isExternalStorageWritable()) {
            // Don't bother writing a separate function to determine if the
            // SD card is readable.
            Log.w("Cardation",
                  "External storage is not mounted, or something");
            return;
        }

        File infile =
            new File(BackupVersion.newestBackupFile(getCurrLanguage()));
        if (!infile.canRead()) {
            Log.e("Cardation", "Cannot read " + infile.getName());
            return;
        }

        InputStream instream = null;
        try {
            instream = new FileInputStream(infile);
        }
        catch(IOException ex) {
            ex.printStackTrace();
            assert(false);
        }

        restoreFromInputStream(instream, context);
    }

    public static void restoreFromInputStream(InputStream instream,
                                              Context context)
    {
        List<List<String>> records = CardationUtils.dbbk2records(instream);

        DbHelper helper = new DbHelper(context);
        SQLiteDatabase db = helper.getReadableDatabase();

        for (int i=0; i<records.size(); ++i) {
            List<String> record = records.get(i);

            String recto = record.get(Contract.recto);
            String verso = record.get(Contract.verso);
            int fwd_streak = Integer.parseInt(record.get(Contract.fwd_streak));
            int bkwd_streak =Integer.parseInt(record.get(Contract.bkwd_streak));
            int due = Integer.parseInt(record.get(Contract.due));
            int importance = Integer.parseInt(record.get(Contract.importance));
            String quote = record.get(Contract.quote);

            Card card = new Card(recto, verso,
                                 fwd_streak, bkwd_streak,
                                 due,
                                 importance,
                                 quote);

            Cursor cursor =
                db.rawQuery("SELECT " + Contract.RECTO + " FROM "
                +   getTableName()
                + " WHERE " + Contract.RECTO + " == "
                + "\"" + recto + "\"", null);
            if (cursor.getCount() == 0) {
                Log.i("Cardation", "Inserting new card " + recto);
                saveCard(context, card);
            } else {
                Log.i("Cardation", "Updating " + recto);
                updateByRecto(context, card);
            }
            cursor.close();
        }
        Log.w("Cardation", "Found " + records.size() + " cards.");

        if (db != null) {
            db.close();
            helper.close();
        }
    }

    public static void deleteLanguage(Context context,
                                      LanguageSpinner spinner) {
        Toast.makeText(context, "Backing up " + getCurrLanguage() + " before "
            + "deleting it", Toast.LENGTH_LONG).show();
        backup(context);

        DbHelper helper = new DbHelper(context);
        SQLiteDatabase db = helper.getWritableDatabase();
        Log.w("Cardation", "Dropping table|" + getTableName() + "|");
        db.execSQL("DROP TABLE IF EXISTS " + getTableName());

        db.close();
        helper.close();

        spinner.remove(getCurrLanguage());
    }

    public static int totalRows(Context context) {
        DbHelper helper = new DbHelper(context);
        SQLiteDatabase db = helper.getReadableDatabase();
        Cursor cursor = db.rawQuery(
                "SELECT COUNT(*) FROM " + getTableName(),
                null);
        cursor.moveToFirst();
        String result_str = cursor.getString(0);
        Log.w("Cardation", "Found " + result_str + " rows in table.");

        db.close();
        helper.close();

        return Integer.parseInt(result_str);
    }
}
