package net.trhj.cardation;

import android.content.Context;
import android.os.Bundle;
import android.app.Activity;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.TextView;
import android.widget.Toast;
import android.util.Log;
import java.io.InputStream;
import java.util.*;

public class RestoreActivity extends Activity {

    // Client sets this, to indicate if we're restoring from the sdcard, or
    // from an email attachment.
    static public String restore_source_ = null;
    static public InputStream restore_attachment_ = null;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_restore);
        getActionBar().setTitle("Restore " + CardDb.getCurrLanguage());
    }

    public void go(View view) {
        FieldOverwrites fo = new FieldOverwrites();
        fo.verso = ((CheckBox) findViewById(R.id.clobberVersoBox)).isChecked();
        fo.quote = ((CheckBox) findViewById(R.id.clobberQuoteBox)).isChecked();
        fo.active =
            ((CheckBox) findViewById(R.id.clobberActiveBox)).isChecked();
        fo.streaks =
            ((CheckBox) findViewById(R.id.clobberStreaksBox)).isChecked();
        fo.duedate =
            ((CheckBox) findViewById(R.id.clobberDuedateBox)).isChecked();

        assert(restore_source_ != null);
        if (restore_source_.equals("sdcard")) {
            assert(restore_attachment_ == null);
            CardDb.restore(this, fo);
        }
        else if (restore_source_.equals("email")) {
            assert(restore_attachment_ != null);
            CardDb.restoreFromInputStream(restore_attachment_, this, fo);
        } else {
            Log.e("Cardation", "Illegal RestoreActivity.restore_source_: "
                + restore_source_ + ", not restoring.");
        }

        ((Button) findViewById(R.id.go_button)).setEnabled(false);
        TextView done = (TextView) findViewById(R.id.done);
        done.setText("done");
    }

    final public class FieldOverwrites {
        public Boolean verso;
        public Boolean quote;
        public Boolean active;
        public Boolean streaks;
        public Boolean duedate;
    }
}
