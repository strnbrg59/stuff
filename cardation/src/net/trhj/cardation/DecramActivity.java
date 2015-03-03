package net.trhj.cardation;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.TextView;
import java.util.ArrayList;
import com.jjoe64.graphview.*;

public class DecramActivity extends Activity
{
    Histogram histogram_ = null;
    ArrayList<Integer> duedates_orig_ = null;
    ArrayList<Integer> duedates_ = null;
    MyNumberPicker cram_picker_;
    int cram_;

    @Override
	protected void onCreate(Bundle savedInstanceState)
    {
		super.onCreate(savedInstanceState);
        getActionBar().setTitle("Decram " + CardDb.getCurrLanguage());

        setContentView(R.layout.activity_decram);
        cram_picker_ = new MyNumberPicker(0, 20, 0,
            (TextView) findViewById(R.id.cramText));

	}

    public void onResume()
    {
        super.onResume();
        Log.i("Cardation", "DecramActivity.onResume()");

        GraphView widget = (GraphView) findViewById(R.id.histogram);
        duedates_orig_ =
            CardDb.getOrderedDueDates(this, CardDb.getCurrLanguage());
        duedates_ = (ArrayList<Integer>)duedates_orig_.clone();
        Log.i("Cardation", "DecramActivity, duedates_.size() = "
            + duedates_.size());
        decramDuedates(cram_picker_.getVal());
        histogram_ = new Histogram(widget, duedates_);
        histogram_.show();
    }

    void decramDuedates(int cram_level)
    {
        duedates_ = (ArrayList<Integer>)duedates_orig_.clone();

        if (cram_level == 0) {
            return;
        }

        final int day_secs = 24*3600;
        final int now = CardationUtils.epochNow();
        int curr_day = now;
        int i = 0;
        int d = duedates_.get(0);
        while (d < now) {
            for (int j=0; j < cram_level; ++j) {
                d = duedates_.get(i);
                if (d >= now) {
                    break;
                }
                duedates_.set(i, curr_day);
                ++i;
            }
            curr_day += day_secs;
        }
        Log.i("Cardation", "Decram: " + (i-1) + " words shifted.");
    }        

    public void cramUp(View view)
    {
        Log.i("Cardation", "DecramActivity.cramUp(), val="
            + cram_picker_.getVal());
        if (cram_picker_.getVal() < cram_picker_.getMax()) {
            // Avoid useless rerendering of the histogram.
            cram_picker_.up();
            cramCommon();
        }
    }
    public void cramDown(View view)
    {
        Log.i("Cardation", "DecramActivity.cramDown()");
        if (cram_picker_.getVal() > cram_picker_.getMin()) {
            cram_picker_.down();
            cramCommon();
        }
    }

    void cramCommon()
    {
        if (duedates_.size() == 0) {
            return;
        }            

        decramDuedates(cram_picker_.getVal());
        histogram_.resetDuedates(duedates_);
        histogram_.show();
    }

    public void commit(View view) {
        Log.i("Cardation", "DecramActivity.commit()");
        CardDb.setOrderedDueDates(duedates_, this, CardDb.getCurrLanguage());
    }
}
