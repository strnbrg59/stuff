package net.trhj.androidflashcards;

import android.os.Bundle;
import android.app.Activity;
import android.view.Menu;
import android.view.View;
import android.content.Intent;
import android.widget.EditText;
import android.util.Log;

public class InputActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_input);
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu); // R.menu.input?
        return true;
    }

    public final static String RECTO = "net.trhj.androidflashcards.RECTO";
    public final static String VERSO = "net.trhj.androidflashcards.VERSO";

    /** Called when the user clicks the Save button */
    public void saveCard(View view) {

        EditText recto = (EditText) findViewById(R.id.recto);
        String recto_str = recto.getText().toString();
        EditText verso = (EditText) findViewById(R.id.verso);
        String verso_str = verso.getText().toString();

        // Save to DB
        long newRowId = DbInteraction.saveCard(recto_str, verso_str,
                                               getBaseContext());
        Log.w("Flashcards", "newRowId = " + newRowId);

        // Blank the EditText's.
        recto.setText("");
        verso.setText("");

        /* Let's save this as an example for using an Intent and passing info
         * on to the next screen.  Even though we don't need it right now.
        Intent intent = new Intent(this, SaveCardActivity.class);

        EditText recto = (EditText) findViewById(R.id.recto);
        String recto_str = recto.getText().toString();
        intent.putExtra(RECTO, recto_str);

        EditText verso = (EditText) findViewById(R.id.verso);
        String verso_str = verso.getText().toString();
        intent.putExtra(VERSO, verso_str);

        startActivity(intent);
        */
    }
}
