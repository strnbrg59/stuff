package net.trhj.cardation;

import android.os.Bundle;
import android.app.Activity;
import android.view.View;
import android.widget.TextView;

public class ConfigActivity extends Activity {

    public static final int default_rand_reversal_ = 10;
    static final int min_rand_reversal_ = 0;
    static final int max_rand_reversal_ = 99;
    MyNumberPicker rand_reversal_picker_;

    public static final int default_batch_size_ = 10;
    static final int min_batch_size_ = 5;
    static final int max_batch_size_ = 100;
    MyNumberPicker batch_size_picker_;

    /* Initial streak */
    public static final int default_streak_ = -3;
    static final int min_streak_ = -10;
    static final int max_streak_ = -1;
    MyNumberPicker streak_picker_;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_config);
        getActionBar().setTitle("Configure " + CardDb.getCurrLanguage());

        rand_reversal_picker_ = new MyNumberPicker(
            min_rand_reversal_,
            max_rand_reversal_,
            ConfigDb.getRandReversal(this),
            (TextView) findViewById(R.id.rand_reversalText));

        batch_size_picker_ = new MyNumberPicker(min_batch_size_,
            max_batch_size_,
            ConfigDb.getBatchSize(this),
            (TextView) findViewById(R.id.batchsizeText));

        streak_picker_ = new MyNumberPicker(min_streak_,
            max_streak_,
            ConfigDb.getInitialStreaks(this),
            (TextView) findViewById(R.id.streakText));
    }

    // It only seems like we don't use the View argument but we do; functions
    // like randReversalUp() and BatchSizeDown() are handlers
    // declared in the xml layout file.
    public void randReversalUp(View view)
    {
        rand_reversal_picker_.up();
        save();
    }
    public void randReversalDown(View view)
    {
        rand_reversal_picker_.down();
        save();
    }

    public void batchsizeUp(View view)
    {
        batch_size_picker_.up();
        save();
    }
    public void batchsizeDown(View view)
    {
        batch_size_picker_.down();
        save();
    }

    public void streakUp(View view)
    {
        streak_picker_.up();
        save();
    }
    public void streakDown(View view)
    {
        streak_picker_.down();
        save();
    }

    private void save()
    {
        ConfigDbFields all = ConfigDb.getDbFieldsForCurrLang(this);
        int batch_size_sign = (int)Math.signum(all.batch_size_);
        int rand_reversal_sign = (int)Math.signum(all.rand_reversal_);
        ConfigDb.update(getBaseContext(),
                        CardDb.getCurrLanguage(),
                        rand_reversal_picker_.getVal() * rand_reversal_sign,
                        batch_size_picker_.getVal() * batch_size_sign,
                        streak_picker_.getVal());
    }
}
