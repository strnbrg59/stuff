package net.trhj.cardation;

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
            // If negative, indicates language is not to show up in the spinner.
        public static final String BATCH_SIZE = "batch_size";
            // If negative, indicates this was the last language, and so
            // on Cardation init, set the language spinner to that language.
        public static final String STREAK = "streak"; // initial streak

        private Contract() {}
    }

    public static String getLastLanguage(Context context)
    {
        DbHelper helper = new DbHelper(context);
        SQLiteDatabase db = helper.getReadableDatabase();
        String query = new String("SELECT * FROM " + table_name_ + " WHERE "
            + Contract.BATCH_SIZE + " < 0");
        Cursor cursor = null;
        String result = "";

        cursor = db.rawQuery(query, null);

        assert(cursor.getCount() <= 1);
        if (cursor.getCount() == 0) {
            Log.w("Cardation",
                "No negative batch-size found (thus no 'last language')");
        } else {
            cursor.moveToFirst();
            result =
                cursor.getString(cursor.getColumnIndex(Contract.LANGUAGE));
        }

        cursor.close();
        db.close();
        helper.close();

        return result;
    }

    //
    // Make it negative -- a hack to avoid having to change the db schema.
    // My old scheme of flipping the sign on curr_language_ whenever we
    // changed curr_language_ was too tricky to get right, between special-
    // casing on init, on language removal, etc.  So instead we'll just go
    // through all the languages and make all the batch sizes positive, except
    // for the batch size of the indicated language.
    //
    public static void setLastLanguageIndicator(Context context,
                                                String language)
    {
        DbHelper helper = new DbHelper(context);
        SQLiteDatabase db = helper.getWritableDatabase();
        String query = new String("SELECT * FROM " + table_name_ + ";");
        Cursor cursor = db.rawQuery(query, null);

        cursor.moveToFirst();
        while (!cursor.isAfterLast()) {
            int lang_col = cursor.getColumnIndex(Contract.LANGUAGE);
            String lang = cursor.getString(lang_col);

            if (lang == null) {
                // Older versions introduced (often lots of) rows in the config
                // db for a "null" language.  Without this fix, we crash on
                // startup.
                Log.w("Cardation", "Found null language, sigh.");
                cursor.moveToNext();
                continue;
            }

            ConfigDbFields all = getDbFields(context, lang);
            all.batch_size_ = Math.abs(all.batch_size_);
            if (lang.equals(language)) {
                Log.i("Cardation", "Setting batch_size neg for lang=" + lang);
                all.batch_size_ *= -1;
            }
            update(context, all);

            cursor.moveToNext();
        }

        cursor.close();
        db.close();
        helper.close();
    }

    static void deleteLanguage(Context context, String lang)
    {
        DbHelper helper = new DbHelper(context);
        SQLiteDatabase db = helper.getReadableDatabase();

        String cmd = "DELETE FROM " + table_name_ + " WHERE "
                   + Contract.LANGUAGE + " = '" + lang + "';";
        Log.i("Cardation", "db.execSQL(" + cmd + ")");
        db.execSQL(cmd);

        db.close();
        helper.close();
    }

    public static ConfigDbFields getDbFieldsForCurrLang(Context context)
    {
        return getDbFields(context, CardDb.getCurrLanguage());
    }

    public static ConfigDbFields getDbFields(Context context, String lang)
    {
        DbHelper helper = new DbHelper(context);
        SQLiteDatabase db = helper.getReadableDatabase();

        String query = new String("SELECT * FROM " + table_name_
                + " WHERE " + Contract.LANGUAGE + " = '" + lang + "'");
        
        Cursor cursor = null;
        ConfigDbFields result = new ConfigDbFields(lang);
        cursor = db.rawQuery(query, null);
        if (cursor.getCount() > 1) {
            Log.e("Cardation", "Multiple config entries ("
                + cursor.getCount() + ") for language " + lang);
            throw new RuntimeException("This is a crash");
        } else if (cursor.getCount() == 0) {
            // On creation of new language.
            return result; // Contains defaults.
        } else {
            cursor.moveToFirst();

            result.language_ = lang;
            result.batch_size_ = Integer.parseInt(
                cursor.getString(cursor.getColumnIndex(
                    Contract.BATCH_SIZE)));
            result.rand_reversal_ = Integer.parseInt(
                cursor.getString(cursor.getColumnIndex(
                    Contract.RAND_REVERSAL)));
            result.initial_streaks_ = Integer.parseInt(
                cursor.getString(cursor.getColumnIndex(
                    Contract.STREAK)));

            cursor.close();
            db.close();
            helper.close();

            return result;
        }
    }

    public static int getBatchSize(Context context)
    {
        return Math.abs(getSignedBatchSize(context));
    }

    public static int getSignedBatchSize(Context context)
    {
        ConfigDbFields all = getDbFieldsForCurrLang(context);
        return all.batch_size_;
    }

    public static int getRandReversal(Context context)
    {
        ConfigDbFields all = getDbFieldsForCurrLang(context);
        return Math.abs(all.rand_reversal_);
    }

    public static Boolean getIsHidden(Context context, String lang)
    {
        ConfigDbFields all = getDbFields(context, lang);
        return (all.rand_reversal_ < 0);
    }

    public static int getInitialStreaks(Context context)
    {
        ConfigDbFields all = getDbFieldsForCurrLang(context);
        return all.initial_streaks_;
    }

    static class DbHelper extends DbCommonHelper {
    
        private static final String DATABASE_NAME = "cardation"
            + Integer.toString(database_version_) + ".db";
        private static Boolean first_time_ = true;

        DbHelper(Context context) {
            super(context, DATABASE_NAME, database_version_);
            if (first_time_) {
                showTableInfo();
                first_time_ = false;
            }
        }
    
        void showTableInfo() {
            Log.i("Cardation", "ConfigDb.DbHelper.showTableInfo():");
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
        public void close() {
            super.close();
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
           Log.w("Cardation",
                 "ConfigDb DB version has changed, but we have no upgrade " +
                 "gameplan as of yet.");
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

    public static void update(Context context, ConfigDbFields all) {
        update(context, all.language_,
               all.rand_reversal_, all.batch_size_, all.initial_streaks_);
    }
}
