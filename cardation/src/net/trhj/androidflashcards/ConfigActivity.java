package net.trhj.androidflashcards;

import android.os.Bundle;
import android.app.Activity;
import android.view.View;
import android.widget.EditText;

public class ConfigActivity extends Activity {

    final static int default_rand_reversal_ = 10;
    static final int min_rand_reversal_ = 0;
    static final int max_rand_reversal_ = 99;
    NumberPicker rand_reversal_picker_;

    final static int default_batch_size_ = 10;
    static final int min_batch_size_ = 5;
    static final int max_batch_size_ = 100;
    NumberPicker batch_size_picker_;

    /* Initial streak */
    final static int default_streak_ = -3;
    static final int min_streak_ = -10;
    static final int max_streak_ = -1;
    NumberPicker streak_picker_;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_config);

        rand_reversal_picker_ = new NumberPicker(
            min_rand_reversal_,
            max_rand_reversal_,
            ConfigDb.getRandReversal(this),
            (EditText) findViewById(R.id.rand_reversalText));

        batch_size_picker_ = new NumberPicker(min_batch_size_,
            max_batch_size_,
            ConfigDb.getBatchSize(this),
            (EditText) findViewById(R.id.batchsizeText));

        streak_picker_ = new NumberPicker(min_streak_,
            max_streak_,
            ConfigDb.getStreak(this),
            (EditText) findViewById(R.id.streakText));
    }

    // It only seems like we don't use the View argument but we do; functions
    // like randReversalUp() and BatchSizeDown() are handlers
    // declared in the xml layout file.
    public void randReversalUp(View view)
    {
        rand_reversal_picker_.up();
    }
    public void randReversalDown(View view)
    {
        rand_reversal_picker_.down();
    }

    public void batchsizeUp(View view)
    {
        batch_size_picker_.up();
    }
    public void batchsizeDown(View view)
    {
        batch_size_picker_.down();
    }

    public void streakUp(View view)
    {
        streak_picker_.up();
    }
    public void streakDown(View view)
    {
        streak_picker_.down();
    }

    public void saveConfig(View view)
    {
        ConfigDb.update(getBaseContext(),
                        CardDb.getCurrLanguage(),
                        rand_reversal_picker_.getVal(),
                        batch_size_picker_.getVal(),
                        streak_picker_.getVal());
    }
}
