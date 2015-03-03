package net.trhj.cardation;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.database.SQLException;
import android.util.Log;
import java.util.ArrayList;

public abstract class DbCommonHelper extends SQLiteOpenHelper {

        DbCommonHelper(Context context, String db_name, int db_version) {
            super(context, db_name, null, db_version);
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

