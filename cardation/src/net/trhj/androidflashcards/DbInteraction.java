/* Based on LoaderThrottleSupport.java and the online database tutorial */

/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package net.trhj.androidflashcards;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.Cursor;
import android.content.ContentValues;
import android.provider.BaseColumns;
import android.util.Log;
import android.os.Environment;
import java.util.Random;
import java.io.File;
import java.io.PrintWriter;

public class DbInteraction {

    /**
     * Definition of the contract for the main table of our provider.
     */
    public static final class CardsContract implements BaseColumns {
        private CardsContract() {}
    
        public static final String TABLE_NAME = "cards";

        public static final String COLUMN_NAME_SUBJECT = "subject";
        public static final String COLUMN_NAME_RECTO = "recto";
        public static final String COLUMN_NAME_VERSO = "verso";

        // These are fields for a future version.  They're not used anywhere
        // for now.
        public static final String COLUMN_NAME_FWDSTREAK = "fwdstreak";
        public static final String COLUMN_NAME_BKWDSTREAK = "bkwdstreak";
        public static final String COLUMN_NAME_LASTDATEFWD = "lastdatefwd";
        public static final String COLUMN_NAME_LASTDATEBKWD = "lastdatebkwd";
        public static final String COLUMN_NAME_IMPORTANCE = "importance";
        public static final String COLUMN_NAME_EXTRA = "extra";
    }

    /**
     * This class helps open, create, and upgrade the database file.
     */
    public static class DatabaseHelper extends SQLiteOpenHelper {
    
        private static final String DATABASE_NAME = "flashcards.db";
        private static final int DATABASE_VERSION = 2;
        private static Boolean first_time_ = true;

        DatabaseHelper(Context context) {
            super(context, DATABASE_NAME, null, DATABASE_VERSION);
            if (first_time_) {
                showTableInfo();
                first_time_ = false;
            }
        }

        void showTableInfo() {
            SQLiteDatabase db = this.getReadableDatabase();
            Cursor cursor = db.rawQuery("PRAGMA TABLE_INFO("
                + CardsContract.TABLE_NAME + ")", null);
            if (cursor.moveToFirst()) {
                do {
                    Log.w("Flashcards", cursor.getString(0) + " " +
                          cursor.getString(1));
                } while (cursor.moveToNext());
            }
            cursor.close();
            db.close();
        }
    
        @Override
        public void onCreate(SQLiteDatabase db) {
            db.execSQL("CREATE TABLE " + CardsContract.TABLE_NAME + " ("
                    + CardsContract._ID + " INTEGER PRIMARY KEY,"
                    + CardsContract.COLUMN_NAME_RECTO + " TEXT" + ","
                    + CardsContract.COLUMN_NAME_VERSO + " TEXT"
                    + ");");
       }
    
       /**
        * A real application should upgrade the database in place.
        */
       @Override
       public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion)
       {
    
           // Logs that the database is being upgraded
           Log.w("Flashcards",
                "Upgrading database from version " + oldVersion + " to "
                   + newVersion + ", which will destroy all old data");
        
           // Kills the table and existing data
           db.execSQL("DROP TABLE IF EXISTS " + CardsContract.TABLE_NAME);
    
           // Recreates the database with a new version
           onCreate(db);
       }
    }
    
    public static long saveCard(String recto, String verso, Context context)
    {
        DatabaseHelper helper = new DatabaseHelper(context);
        SQLiteDatabase db = helper.getWritableDatabase();
        ContentValues values = new ContentValues();
        values.put(CardsContract.COLUMN_NAME_RECTO, recto);
        values.put(CardsContract.COLUMN_NAME_VERSO, verso);
        long newRowId =
            db.insert(CardsContract.TABLE_NAME, null, values);

        db.close();
        helper.close();
        return newRowId;
    }

    public final static class Card {
        public Card(String recto, String verso) {
            recto_ = recto;
            verso_ = verso;
        }
        public final String recto_;
        public final String verso_;
    }
    
    public static Card getRandomCard(Context context)
    {
        DatabaseHelper helper = new DatabaseHelper(context);
        SQLiteDatabase db = helper.getReadableDatabase();
        String[] projection = {
            CardsContract._ID,
            CardsContract.COLUMN_NAME_RECTO,
            CardsContract.COLUMN_NAME_VERSO};
        Cursor cursor = db.query(CardsContract.TABLE_NAME,
                                 projection,
                                 null, // selection
                                 null, // selectionArgs
                                 null, // groupBy
                                 null, // having
                                 null // orderBy
                                );

        int n_rows = cursor.getCount();
        if (n_rows == 0) {
            Log.w("FlashCards", "n_rows = 0 in getRandomCard()");
        }
        assert(n_rows > 0);
        Random generator = new Random();
        cursor.moveToPosition(generator.nextInt(n_rows));
        int recto_column =
            cursor.getColumnIndexOrThrow(CardsContract.COLUMN_NAME_RECTO);
        int verso_column =
            cursor.getColumnIndexOrThrow(CardsContract.COLUMN_NAME_VERSO);

        db.close();
        helper.close();
        return new Card(cursor.getString(recto_column),
                        cursor.getString(verso_column));
    }

    public static boolean isExternalStorageWritable()
    {
        String state = Environment.getExternalStorageState();
        if (Environment.MEDIA_MOUNTED.equals(state)) {
            return true;
        }
        return false;
    }



    public static void backup(Context context) {
        Log.w("FlashCards", "DbInteraction.backup() here");
        if (!isExternalStorageWritable()) {
            Log.w("FlashCards", "External storage is not writable");
            return;
        }

        String package_name = new String("net.trhj.androidflashcards");
        String special_dir = new String("Android/data/data/" + package_name);

        File outfile = new File(
          Environment.getExternalStorageDirectory() + "/db.bk");

        Log.w("FlashCards",
            "outfile.getAbsolutePath() = " + outfile.getAbsolutePath());
/*
        if (!outfile.mkdirs()) {
            Log.e("FlashCards", "Failed to mkdir ");
            return;
        }
*/
        Log.w("FlashCards", "outfile.isFile() = " + outfile.isFile());
        Log.w("FlashCards", "outfile.isDirectory() = " + outfile.isDirectory());
        Log.w("FlashCards", "outfile.canWrite() = " + outfile.canWrite());

        // Now go through the entire DB and print it out.
        try {
            PrintWriter out = new PrintWriter(outfile.getAbsolutePath());
            out.println("foo bar");
            out.close();
        }
        catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void restore() {
        Log.w("FlashCards", "DbInteraction.restore() here, no-op still");
    }
}
