/* Based on LoaderThrottleSupport.java and the online database tutorial */

package net.trhj.cardation;

import android.content.Intent;
import android.content.Context;
import android.content.ContentValues;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.database.SQLException;
import android.net.Uri;
import android.os.Environment;
import android.provider.BaseColumns;
import android.util.Log;
import android.widget.Toast;
import java.io.*;
import java.lang.Math;
import java.util.ArrayList;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;
import java.util.Random;
import java.util.StringTokenizer;
import java.util.Vector;

public class CardDb {

    private static final int database_version_ = 8;
    public static String pkg_underbars_ = "net_trhj_cardation";
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
        // Called from LearnActivity to report in info area.
        return n_due_;
    }


    static public int getNDue(String lang, Context context)
    {
        // Forces a look into the DB -- expensive.
        LinkedList<Card> dues = getDueCards(context, Integer.MAX_VALUE, lang);
        return dues.size();
    }

    static public void setCurrLanguage(String lang, Context context)
    {
        assert(lang != null);
        curr_language_ = lang;
        Log.i("Cardation", "curr_language_ set to " + lang);

        ConfigDb.setLastLanguageIndicator(context, lang);

        getNDue(lang, context);
        // Executed solely for the side effect of updating this.n_due_.
    }

    static public String getCurrLanguage()
    {
        return curr_language_;
    }

    static public String getCurrentTableName()
    {
        return tableName(curr_language_);
    }

    static String tableName(String lang)
    {
        if (lang == null) {
            Log.e("Cardation", "curr_language_ = null !?!");
        }
        return pkg_underbars_ + "_" + lang;
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

    public static void createTable(String lang, Context context) {
        DbHelper db = new DbHelper(context);
        db.createTable(lang);
    }

    /**
     * This class helps open, create, and upgrade the database file.
     */
    static class DbHelper extends SQLiteOpenHelper {
    
        private static final String DATABASE_NAME = "cardation"
            + Integer.toString(database_version_) + ".db";
        Context context_;
    
        DbHelper(Context context) {
            super(context, DATABASE_NAME, null, database_version_);
            Log.d("Cardation", "CardDb.DbHelper ctor");
            context_ = context;
        }

        @Override
        public void onCreate(SQLiteDatabase db) {
        }

        @Override
        public void close() {
            super.close();
            Log.d("Cardation", "CardDb.DbHelper.close() called");
        }
    
        @Override
        public SQLiteDatabase getReadableDatabase() {
            SQLiteDatabase db = super.getReadableDatabase();
            return db;
        }

        @Override
        public SQLiteDatabase getWritableDatabase() {
            SQLiteDatabase db = super.getWritableDatabase();
            return db;
        }

        public void createTable(String lang) {
            SQLiteDatabase db = super.getWritableDatabase();
            db.execSQL("CREATE TABLE " + tableName(lang) + " ("
                + Contract.RECTO + " TEXT PRIMARY KEY,"
                + Contract.VERSO + " TEXT" + ","
                + Contract.FWD_STREAK + " INTEGER" + ","
                + Contract.BKWD_STREAK + " INTEGER" + ","
                + Contract.DUE + " INTEGER" + ","
                + Contract.IMPORTANCE + " INTEGER" + ","
                + Contract.QUOTE + " TEXT"
                + ");");
        }
    
       /**
        * A real application should upgrade the database in place.
        */
       @Override
       public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion)
       {
           Log.w("Cardation",
                 "CardDb DB version has changed, but we have no upgrade " +
                 "gameplan as of yet.");
       }

        //
        // This comes from
        // https://github.com/sanathp/DatabaseManager_For_Android
        //
        public ArrayList<Cursor> getData(String Query){
        	//get writable database
        	SQLiteDatabase sqlDB = this.getWritableDatabase();
        	String[] columns = new String[] { "mesage" };
        	// An array list of cursor to save two cursors.  One has results
            // from the query, the other stores error message if any errors are
            // triggered.
        	ArrayList<Cursor> alc = new ArrayList<Cursor>(2);
        	MatrixCursor Cursor2= new MatrixCursor(columns);
        	alc.add(null);
        	alc.add(null);
        	
        	
        	try{
        		String maxQuery = Query ;
        		//execute the query results will be save in Cursor c
        		Cursor c = sqlDB.rawQuery(maxQuery, null);
        		
        
        		//add value to cursor2
        		Cursor2.addRow(new Object[] { "Success" });
        		
        		alc.set(1,Cursor2);
        		if (null != c && c.getCount() > 0) {
        
        			
        			alc.set(0,c);
        			c.moveToFirst();
        			
        			return alc ;
        		}
        		return alc;
        	} catch(SQLException sqlEx){
        		Log.d("printing exception", sqlEx.getMessage());
        		// If any exceptions are triggered save the error message to
                // cursor and return the arraylist.
        		Cursor2.addRow(new Object[] { ""+sqlEx.getMessage() });
        		alc.set(1,Cursor2);
        		return alc;
        	} catch(Exception ex){
        
        		Log.d("printing exception", ex.getMessage());
        
        		// If any exceptions are triggered save the error message to
                // cursor an return the arraylist.
        		Cursor2.addRow(new Object[] { ""+ex.getMessage() });
        		alc.set(1,Cursor2);
        		return alc;
        	}
        
        	
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
            db.insert(getCurrentTableName(), null, values);

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
            String language = extractLanguageTokenFromDbTableName(table_name);
            Log.w("Cardation", "language = |" + language + "|");
            if (language != null) {
                // E.g. net_trhj_cardation_Polish.  For now, let's disallow
                // "languages" with underscores in their names.
                // The only non-language table we're likely to see is
                // android_metadata.
                Log.i("Cardation", "CardDb.listLanguages(), adding " +language);
                result.add(language);
            }

            cursor.moveToNext();
        }

        cursor.close();
        db.close();
        helper.close();
        return result;
    }

    static public String extractLanguageTokenFromDbTableName(String table_name)
    {
        String result = null;
        StringTokenizer toker = new StringTokenizer(table_name, "_");
        int n_tokens = toker.countTokens();
        Log.i("Cardation", "extractLanguage(), n_tokens=" + n_tokens);
        if ((n_tokens == 4) || (n_tokens == 5)) {
            // 4 if it's a table, 5 if it's a backup file (5th token is a
            // numeral).
            for (int i=0; i<3; ++i) toker.nextToken();
            result = toker.nextToken();
            Log.i("Cardation", "extractLanguage(), result=|" + result + "|");
        }

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

    public static LinkedList<Card> getDueCards(Context context, int max_cards)
    {
        return getDueCards(context, max_cards, curr_language_);
    }


    //
    // Find cards whose due date is earlier than now.  Limit the batch to
    // max_cards, to keep the work load reasonable.
    //
    public static LinkedList<Card> getDueCards(Context context,
        int max_cards, String language)
    {
        DbHelper helper = new DbHelper(context);
        SQLiteDatabase db = helper.getReadableDatabase();
        String query = new String("SELECT * FROM " + tableName(language)
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
            result.push(newCard);
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
            getCurrentTableName() + " WHERE " + Contract.RECTO +
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

        int count = db.update(getCurrentTableName(),
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
        db.delete(getCurrentTableName(), selection, selectionArgs);
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
        Cursor cursor = db.rawQuery("SELECT * FROM " + getCurrentTableName(),
                                    null);
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


    //
    // Back up all languages.  Send one email with a separate attachment for
    // each language.
    //
    public static void backup(Context context) {
        DbHelper helper = new DbHelper(context);
        SQLiteDatabase db = helper.getReadableDatabase();
        assert(db != null);

        // Set up email (content of attachments built later).
        ArrayList<Uri> attachments = new ArrayList<Uri>();
        Intent intent = new Intent(Intent.ACTION_SEND_MULTIPLE);
        intent.setType("message/rfc822");
        intent.putExtra(Intent.EXTRA_EMAIL,
                        new String[]{"strnbrg59@gmail.com"});
        intent.putExtra(Intent.EXTRA_SUBJECT, "Cardation full backup");

        // Write a backup file for each language, on /mnt/sdcard.
        // Attach all those files to the email.
        LinkedList<String> langs = listLanguages(context);
        for (int i=0; i < langs.size(); i++) {
            String lang = langs.get(i);

            Cursor cursor = db.rawQuery(
                "SELECT * FROM " + tableName(lang), null);
            String[] col_names = cursor.getColumnNames();
            StringBuffer everythingBuf = new StringBuffer();
            for (int r=0; r<cursor.getCount(); ++r) {
                cursor.moveToPosition(r);
                for (int c=0; c<col_names.length; ++c) {
                    String delimiter = "@";
                    everythingBuf.append(cursor.getString(c));
                    everythingBuf.append(delimiter);
                }
                everythingBuf.append('\n');
            }
            String everything = everythingBuf.toString();
            cursor.close();

            Log.w("Cardation", "Preparing to backup " + cursor.getCount()
                  + " records.");

            /*
             * Write a backup file on /mnt/sdcard.  Typically something like
             * /mnt/sdcard/net_trhj_cardation_Spanish_4.dbbk.
             */
            if (!isExternalStorageWritable()) {
                Log.w("Cardation", "External storage is not writable");
                return;
            }

            // Compiler insists we put this in a try-catch block:
            String backup_filename = BackupVersion.nextBackupFile(lang);
            try {
                File outfile = new File(backup_filename);
                Log.i("Cardation",
                    "outfile.getAbsolutePath() = " + outfile.getAbsolutePath());
                Log.i("Cardation",
                      "outfile.canWrite() = " + outfile.canWrite());

                OutputStream outstream = new FileOutputStream(outfile);
                BufferedWriter writer =
                    new BufferedWriter(new OutputStreamWriter(outstream,
                                                              "UTF-8"));
                writer.write(everything);
                writer.close();
                Log.w("Cardation", "Backed up " + outfile.getAbsolutePath());
            } catch (Exception ex) {
                Log.e("Cardation", "Could not write backup to /mnt/sdcard.");
            }

            // Attach backup file to email.
            Log.i("Cardation", "intent.putExtra(" + backup_filename + ")");
            File infile = new File(backup_filename);
            Uri uri = Uri.parse("file://" + infile);
            attachments.add(uri);
        }

        // Attachments attached, time to send out the email:
        intent.putParcelableArrayListExtra(Intent.EXTRA_STREAM, attachments);
        try {
            context.startActivity(Intent.createChooser(intent, "Send mail..."));
        } catch (android.content.ActivityNotFoundException ex) {
            Toast.makeText(context,
            "There are no email clients installed.", Toast.LENGTH_LONG).show();
        } catch(Exception ex) {
            Log.w("Cardation", "Could not send out email, ok.");
        }

        db.close();
        helper.close();
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

        assert(instream != null);
        restoreFromInputStream(instream, context);
    }

    public static void restoreFromInputStream(InputStream instream,
                                              Context context)
    {
        List<List<String>> records = CardationUtils.dbbk2records(instream);
        if (records == null) {
            Toast.makeText(context, "Bad or empty dbbk file",
                       Toast.LENGTH_LONG).show();
            System.exit(0);
        }

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

            recto = recto.replace("'", "''"); // That's how you escape a quote.
            Cursor cursor =
                db.rawQuery("SELECT " + Contract.RECTO + " FROM "
                +   getCurrentTableName()
                + " WHERE " + Contract.RECTO + " == "
                + "\'" + recto + "\'", null);
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

    public static void deleteCurrentLanguage(Context context,
                                             LanguageSpinner spinner) {
        Toast.makeText(context, "Doing complete backup first...",
                       Toast.LENGTH_LONG).show();
        backup(context);

        DbHelper helper = new DbHelper(context);
        SQLiteDatabase db = helper.getWritableDatabase();
        Log.w("Cardation", "Dropping table|" + getCurrentTableName() + "|");
        db.execSQL("DROP TABLE IF EXISTS " + getCurrentTableName() + ";");

        db.close();
        helper.close();

        // Delete from ConfigDb *after* CardDb, or else with
        // CardDb.curr_language_ still equal to the language we're in the
        // process of deleting, ConfigDb will recreate its entry.
        ConfigDb.deleteLanguage(context, getCurrLanguage());

        spinner.remove(getCurrLanguage());
    }

    public static int totalRows(Context context) {
        DbHelper helper = new DbHelper(context);
        SQLiteDatabase db = helper.getReadableDatabase();
        Cursor cursor = db.rawQuery(
                "SELECT COUNT(*) FROM " + getCurrentTableName(),
                null);
        cursor.moveToFirst();
        String result_str = cursor.getString(0);
        Log.w("Cardation", "Found " + result_str + " rows in table "
              + getCurrentTableName());

        db.close();
        helper.close();

        return Integer.parseInt(result_str);
    }

    public static Vector<ComplexInt> duedateHistogram(Context context,
                                                      String lang) {
        DbHelper helper = new DbHelper(context);
        SQLiteDatabase db = helper.getReadableDatabase();
        Vector<ComplexInt> result = new Vector<ComplexInt>();
        String query_template = "SELECT COUNT(*) FROM " + tableName(lang)
            + " WHERE (%s > %d) AND (%s <= %d);";

        final int now = CardationUtils.epochNow();
        int later_than = 0;
        int earlier_than = now;

        // Make histogram with power-of-two time categories.
        final int day_seconds = 24*3600;
        final int n_bins = 12;
        for (int x=0; x<=n_bins-2; ++x) {
            String query = String.format(query_template,
                Contract.DUE, later_than, Contract.DUE, earlier_than);
            Log.i("Cardation", "histogram query = " + query);
            Cursor cursor = db.rawQuery(query,null);
            cursor.moveToFirst();
            int count = Integer.parseInt(cursor.getString(0));

            result.add(new ComplexInt(x-1, count));

            later_than = earlier_than;
            if (x == n_bins-2) {
                earlier_than = Integer.MAX_VALUE;
            } else {
                earlier_than += (int)(Math.pow(2,x))*day_seconds;
            }
        }

        db.close();
        helper.close();

        return result;
    }
}
