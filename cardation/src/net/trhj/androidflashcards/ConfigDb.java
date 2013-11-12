package net.trhj.androidflashcards;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.Cursor;
import android.content.ContentValues;
import android.provider.BaseColumns;
import android.util.Log;

public class ConfigDb {

    public static final String table_name_ = "configuration";
    private static final int database_version_ = 4;

    public static final class Contract implements BaseColumns {
        public static final String LANGUAGE = "language";
        public static final String RAND_REVERSAL
            = "rand_reversal";
        public static final String BATCH_SIZE = "batch_size";
        public static final String STREAK = "streak";

        private Contract() {}
    }

    /* Guts of getBatchSize() and getRandReversal() */
    private static Cursor getRawQueryCursor(Context context)
    {
        DbHelper helper = new DbHelper(context);
        SQLiteDatabase db = helper.getReadableDatabase();
        String lang = CardDb.getCurrLanguage();

        String query = new String("SELECT * FROM " + table_name_
                + " WHERE " + Contract.LANGUAGE + " = '" + lang + "'");
        
        Cursor cursor = null;
        try {
            cursor = db.rawQuery(query, null);
            cursor.moveToFirst();
            do {
                String msg = "Retrieved Config DB record: ";
                for (int i=0; i<cursor.getColumnCount(); ++i) {
                    msg += "|" + cursor.getString(i) + "| ";
                }
                Log.w("Cardation", msg);
            } while (cursor.moveToNext());
        }
        catch (RuntimeException ex) {
            Log.w("Cardation", "Config for |" + lang + "| not found.");
            update(context, lang,
                   ConfigActivity.default_rand_reversal_,
                   ConfigActivity.default_batch_size_,
                   ConfigActivity.default_streak_);
            return null;
        }
        
        if (cursor.getCount() > 1) {
            Log.e("Cardation", "Multiple config entries (" + cursor.getCount() +
                  ") for language " + lang);
            throw new RuntimeException("This is a crash");
        } else {
            db.close();
            helper.close();
            return cursor;
        }
    }

    public static int getBatchSize(Context context)
    {
        Cursor cursor = getRawQueryCursor(context);
        if (cursor == null) {
            return ConfigActivity.default_batch_size_;
        }
        else {
            cursor.moveToPosition(0);
            String result =
                cursor.getString(cursor.getColumnIndex(Contract.BATCH_SIZE));
            cursor.close();
            return Integer.parseInt(result);
        }
    }

    public static int getRandReversal(Context context)
    {
        Cursor cursor = getRawQueryCursor(context);
        if (cursor == null) {
            return ConfigActivity.default_rand_reversal_;
        }
        else {
            cursor.moveToPosition(0);
            int result = Integer.parseInt(
                cursor.getString(cursor.getColumnIndex(
                    Contract.RAND_REVERSAL)));
            cursor.close();
            return result;
        }
    }

    public static int getStreak(Context context)
    {
        Cursor cursor = getRawQueryCursor(context);
        if (cursor == null) {
            return ConfigActivity.default_streak_;
        }
        else {
            cursor.moveToPosition(0);
            int result = Integer.parseInt(
                cursor.getString(cursor.getColumnIndex(Contract.STREAK)));
            cursor.close();
            return result;
        }
    }

    static class DbHelper extends SQLiteOpenHelper {
    
        private static final String DATABASE_NAME = "cardation"
            + Integer.toString(database_version_) + ".db";
        private static Boolean first_time_ = true;

        Context context_;
    
        DbHelper(Context context) {
            super(context, DATABASE_NAME, null, database_version_);
            context_ = context;
            if (first_time_) {
                showTableInfo();
                first_time_ = false;
            }
        }
    
        void showTableInfo() {
            SQLiteDatabase db = this.getReadableDatabase();
            Cursor cursor = db.rawQuery("select * from " + table_name_, null);
            if (cursor.moveToFirst()) {
                do {
                    String columns = "";
                    for (int i=0; i<cursor.getColumnCount(); ++i) {
                        columns += cursor.getString(i) + " ";
                    }
                    Log.w("Cardation", columns);
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
            db.execSQL("CREATE TABLE IF NOT EXISTS " + table_name_ + " ("
                + Contract._ID + " INTEGER PRIMARY KEY,"
                + Contract.LANGUAGE + " TEXT" + ","
                + Contract.RAND_REVERSAL + " INTEGER" + ","
                + Contract.BATCH_SIZE + " INTEGER" + ","
                + Contract.STREAK + " INTEGER"
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
    
           // Logs that the database is being upgraded
           Log.w("Cardation",
                "Upgrading database from version " + oldVersion + " to "
                   + newVersion + ", which will destroy all old data");
        
           // Kills the table and existing data
           db.execSQL("DROP TABLE IF EXISTS " + table_name_);
    
           // Recreates the database with a new version
           onCreate(db);
       }
    }
    
    /* Or insert, if key (language) doesn't exist. */
    public static void update(Context context,
                              String language,
                              int rand_reversal,
                              int batch_size,
                              int streak)
    {
        DbHelper helper = new DbHelper(context);
        SQLiteDatabase db = helper.getWritableDatabase();
     
        ContentValues values = new ContentValues();
        values.put(Contract.LANGUAGE, language);
        values.put(Contract.RAND_REVERSAL, rand_reversal);
        values.put(Contract.BATCH_SIZE, batch_size);
        values.put(Contract.STREAK, streak);

        String selection = Contract.LANGUAGE + " LIKE ?";
        String[] selectionArgs = { language };

        int count = db.update(table_name_,
                              values,
                              selection,
                              selectionArgs);
        if (count == 0) {
            db.insert(table_name_, null, values);
        }

        db.close();
        helper.close();
    }
}
