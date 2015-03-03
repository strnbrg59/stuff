package net.trhj.cardation;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.graphics.Color;
import android.app.Activity;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.Toast;
import android.util.Log;
import java.util.*;
import java.lang.Math;

public class LearnActivity extends Activity {

    static class G {
        // Initialized in onResume()
        static CardDb.Card curr_card=null;
        static LinkedList<CardDb.Card> batch=null;
        static Stack<CardDb.Card> prev_cards=null;
        static boolean forward;
        static int total_rows;
        static boolean clicked_yesnomeh_already;
        static int max_batch_size;
        static int rand_reversal;
        static MyNumberPicker cram_picker_=null;
        static String default_quote_;

        static int enabled_text_color_;
    }

	@Override
    @SuppressWarnings("unchecked")
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_learn);
        final Intent intent = new Intent(this, DecramActivity.class);
        Button cramdown_button = (Button) findViewById(R.id.cramDownButton);
        cramdown_button.setOnLongClickListener(new View.OnLongClickListener() {
            @Override
            public boolean onLongClick(View v) {
                if (G.cram_picker_.getVal() == 0) {
                    startActivity(intent);
                }
                return true;
            }
        });
	}

    public void onResume() {
        super.onResume();
        Log.i("Cardation", "LearnActivity.onResume()");

        EditText recto = (EditText) findViewById(R.id.recto);
        G.enabled_text_color_ = recto.getCurrentTextColor();

        G.curr_card = null;
        G.batch = null;
        G.prev_cards = new Stack<CardDb.Card>();
        G.forward = true;
        G.total_rows = 0;
        G.clicked_yesnomeh_already = false;
        G.max_batch_size = 0;
        G.rand_reversal = 0;
        G.cram_picker_ = null;
        G.default_quote_ = "---";
        G.cram_picker_ = new MyNumberPicker(0, 8, 0,
            (TextView) findViewById(R.id.cramText));

        //
        // FIXME: at this point, CardDb.curr_language_ might be null (after
        // returning back from another app -- not on Cardation start because
        // then we just came out of the language spinner) which means that
        // when we come back into LearnActivity, suddenly we're looking at a
        // small card deck associated with the null language (and of course the
        // fact we have any words in the null language at all is some kind of
        // other bug, whose cause I haven't even thought about yet).
        //

        CardDb.setCramIncrement(G.cram_picker_.getVal());
        G.total_rows = CardDb.totalRows(getBaseContext());
        G.max_batch_size = ConfigDb.getBatchSize(this);
        G.rand_reversal = ConfigDb.getRandReversal(this);
        G.batch = CardDb.getDueCards(this, G.max_batch_size);

        getActionBar().setTitle("Learn " + CardDb.getCurrLanguage());

        advanceCard();
    }

    public void onPause() {
        super.onPause();
        Log.i("Cardation", "LearnActivity.onPause()");
    }

    public void onStop() {
        super.onStop();
        Log.i("Cardation", "LearnActivity.onStop()");
    }

    public static int getMinStreak(Context context) {
        return ConfigDb.getInitialStreaks(context);
    }

    public void cramUp(View view)
    {
        G.cram_picker_.up();
        cramChangeCommon();
    }
    public void cramDown(View view)
    {
        if (G.cram_picker_.getVal() >= 0) {
            G.cram_picker_.down();
        }
            
        cramChangeCommon();
    }

    void cramChangeCommon()
    {
        CardDb.setCramIncrement(G.cram_picker_.getVal());
        G.batch.clear();
        resample();
        advanceCard();
    }

    public void goBack(View view)
    {
        if (G.prev_cards.empty()) {
            Log.d("Cardation", "prev_cards is empty");
            Toast.makeText(this, "Nothing to go back to",
                           Toast.LENGTH_LONG).show();
        } else {
            G.batch.push(G.prev_cards.pop());
            G.clicked_yesnomeh_already = false;
            if (G.batch.size() == 1) {
                enableWidgets();
            }
            advanceCard();
        }
    }


    /* When cards drop out of G.batch, we replace them with other cards
     * that are now due.
     */
    @SuppressWarnings("unchecked")
    void resample() {
        LinkedList<CardDb.Card> all_due =
            CardDb.getDueCards(this, G.max_batch_size);
        Log.w("Cardation", "resampled, found " + all_due.size() + " due cards");

        while (    (all_due.size() > 0)
                && (G.batch.size() < G.max_batch_size)) {
            CardDb.Card candidate_card = all_due.pop();

            // Don't push duplicates.
            if (CardationUtils.batchContainsRecto(G.batch,
                                                  candidate_card.recto_)) {
                continue;
            }

            G.batch.addLast(candidate_card);
        }

        if (G.batch.size() > 0) {
            enableWidgets();
        }
    }


    public void advanceCard() {
        if (G.batch.size() == 0) {
            resample();
        }
        if (G.batch.size() == 0) {
            // Definitely done now.
            Log.w("Cardation", "Empty batch, no cards are due now.");
            disableWidgets();

            // We're done, but a dummy card is helpful here.
            G.curr_card = null;
        } else {
            G.curr_card = G.batch.pop();
        }

        // Populate widgets.
        EditText recto = (EditText) findViewById(R.id.recto);
        EditText verso = (EditText) findViewById(R.id.verso);
        EditText quote = (EditText) findViewById(R.id.quote);
        CheckBox active = (CheckBox) findViewById(R.id.activeCheckBox);
        EditText info = (EditText) findViewById(R.id.infoEditText);

        String info_msg = new String();
        info.setEnabled(false);
        int batch_size;
        if (G.curr_card == null) {
            batch_size = 0;
            recto.setText("---");
            verso.setText("---");
        } else {
            batch_size = G.batch.size() + 1; // Just did that pop()
            G.forward = CardationUtils.shouldGoForward(
                G.curr_card.importance_ == 1,
                G.curr_card.fwd_streak_,
                G.rand_reversal/100.0);
            if (G.forward == true) {
                recto.setText(G.curr_card.recto_);
                verso.setText("?");
            } else {
                verso.setText(G.curr_card.verso_);
                recto.setText("?");
            }
            active.setChecked(G.curr_card.importance_ == 1);
            info_msg += "streak = (" +
                Integer.toString(G.curr_card.fwd_streak_) + ", " +
                Integer.toString(G.curr_card.bkwd_streak_) + ")"
                + '\n';
        }
        quote.setText(G.default_quote_);
        info_msg += batch_size + " in batch.";
        info_msg += "\n" + CardDb.getNDue() + " now due.";
        info_msg += "\n" + "Total in DB = " + G.total_rows;
        info.setText(info_msg);
        info.setTextColor(G.enabled_text_color_);
        Log.i("Cardation", "G.enabled_text_color_ = "
            + Integer.toHexString(G.enabled_text_color_ & 0x00FFFFFF));
    }

    /** Display the verso and the quote */
    public void showAnswer(View view) {
        assert(G.curr_card != null);
        if (G.forward == true) {
            EditText verso = (EditText) findViewById(R.id.verso);
            verso.setText(G.curr_card.verso_);
        } else {
            EditText recto = (EditText) findViewById(R.id.recto);
            recto.setText(G.curr_card.recto_);
        }
        EditText quote = (EditText) findViewById(R.id.quote);
        quote.setText(G.curr_card.quote_);
    }

    // 
    // Store the updated streaks and due dates.
    // If anything on the card was edited, store that too.
    //
    public void updateCard(View view, int knew_it) {
        CardDb.Card updated_card = (CardDb.Card)G.curr_card.clone();

        //
        // Update the streaks.
        //
        if (knew_it == 1) {                 // yes
            if (G.forward == true) {
                updated_card.fwd_streak_ += 1;
            } else {
                updated_card.bkwd_streak_ += 1;
            }
        } else if (knew_it == -1) {         //no
            if (G.forward == true) {
                updated_card.fwd_streak_ =
                    Math.min(-1,
                             Math.max(getMinStreak(this),
                                      updated_card.fwd_streak_ - 1));
            } else {
                if (updated_card.bkwd_streak_ == getMinStreak(this)) {
                    // Give up on backward for a while.  Prepare the ground to
                    // go forward next time.
                    updated_card.fwd_streak_ = updated_card.bkwd_streak_
                        = getMinStreak(this);
                } else {
                    updated_card.bkwd_streak_ =
                        Math.min(-1,
                                 Math.max(getMinStreak(this),
                                          updated_card.bkwd_streak_ - 1));
                }
            }
        } else if (knew_it == 0) {          // meh
            // Halve the streak, if it's positive.
            if (G.forward == true) {
                if (updated_card.fwd_streak_ > 0) {
                    updated_card.fwd_streak_ /= 2;
                }
            } else {
                if (updated_card.bkwd_streak_ > 0) {
                    updated_card.bkwd_streak_ /= 2;
                }
            }
        }

        //
        // Update the widgets.
        //

        // Update recto, verso and quote in case they've been edited.
        EditText recto = (EditText) findViewById(R.id.recto);
        updated_card.recto_ = recto.getText().toString();
        if (updated_card.recto_.equals("?")) {
            updated_card.recto_ = G.curr_card.recto_;
        }
        EditText verso = (EditText) findViewById(R.id.verso);
        updated_card.verso_ = verso.getText().toString();
        if (updated_card.verso_.equals("?")) {
            updated_card.verso_ = G.curr_card.verso_;
        }
        EditText quote = (EditText) findViewById(R.id.quote);
        updated_card.quote_ = quote.getText().toString();
        if (updated_card.quote_.equals(G.default_quote_)) {
            updated_card.quote_ = G.curr_card.quote_;
        }

        // Delete the card if so indicated.
        if (updated_card.recto_.equals("")) {
            CardDb.deleteCard(this, G.curr_card);
            Toast.makeText(this, "Deleted " + G.curr_card.recto_,
                           Toast.LENGTH_LONG).show();
            return;
        }

        // Importance/active checkbox.
        if (((CheckBox) findViewById(R.id.activeCheckBox)).isChecked()) {
            updated_card.importance_ = 1;
        } else {
            updated_card.importance_ = 0;
        }


        // We never needed two due-date columns, but let's not monkey with the
        // DB's schema now or we'll need to do a custom restore.
        int due = CardationUtils.dueDate(updated_card.fwd_streak_,
                                         updated_card.bkwd_streak_,
                                         updated_card.importance_==1);
        int now = CardationUtils.epochNow();
        Log.w("Cardation", "due in " +
            (due - now)/(24.0*3600) + " days from now.");
        updated_card.due_ = due;

        if (G.curr_card.recto_.equals(updated_card.recto_)) {
            CardDb.saveCard(getBaseContext(), updated_card);
        } else { // Must have edited recto.
            CardDb.deleteCard(getBaseContext(), G.curr_card);
            if (updated_card.recto_.equals("")) {
                Toast.makeText(this, "Deleted " + G.curr_card.recto_,
                               Toast.LENGTH_LONG).show();
            } else {
                CardDb.saveCard(getBaseContext(), updated_card);
                Toast.makeText(this, "Modified recto, ok",
                               Toast.LENGTH_LONG).show();
                Log.w("Cardation", "Changed recto from " + G.curr_card.recto_
                    +" to " + updated_card.recto_);
            }
        }

        if (due > now) {
            // I.e. if curr_card is now no longer due.
            resample();
        }

        G.prev_cards.push(updated_card);
    }


    void disableWidgets() {
        EditText recto = (EditText) findViewById(R.id.recto);
        recto.setEnabled(false);
        EditText verso = (EditText) findViewById(R.id.verso);
        verso.setEnabled(false);
        EditText quote = (EditText) findViewById(R.id.quote);
        quote.setEnabled(false);
        CheckBox active = (CheckBox) findViewById(R.id.activeCheckBox);
        active.setEnabled(false);
        RadioGroup rg = (RadioGroup)findViewById(R.id.know_radio_group);
        for (int i=0; i<rg.getChildCount(); ++i) {
            ((RadioButton) rg.getChildAt(i)).setEnabled(false);
        }
    }

    void enableWidgets() {
        EditText recto = (EditText) findViewById(R.id.recto);
        recto.setEnabled(true);
        EditText verso = (EditText) findViewById(R.id.verso);
        verso.setEnabled(true);
        EditText quote = (EditText) findViewById(R.id.quote);
        quote.setEnabled(true);
        CheckBox active = (CheckBox) findViewById(R.id.activeCheckBox);
        active.setEnabled(true);
        RadioGroup rg = (RadioGroup)findViewById(R.id.know_radio_group);
        for (int i=0; i<rg.getChildCount(); ++i) {
            ((RadioButton) rg.getChildAt(i)).setEnabled(true);
            // This is another attempt, failed, to make the label show up, on
            // Android 2.2.2:
            ((RadioButton) rg.getChildAt(i)).setTextColor(Color.BLACK);
        }
    }


    public void onRadioButtonClicked(View view) {
        // To advance the card, you need to click twice.  On the first click,
        // all that happens is that the verso and the recto are displayed.
        G.clicked_yesnomeh_already ^= true;
        if (G.clicked_yesnomeh_already) {
            showAnswer(view);
            return;
        }

        boolean checked = ((RadioButton) view).isChecked();
        int knew_it = 0; // -1=no, 0=meh, 1=yes

        switch(view.getId()) {
            case R.id.know_button_no:
                if (checked) {
                    knew_it = -1;
                }
                break;
            case R.id.know_button_meh:
                if (checked) {
                    knew_it = 0;
                }
                break;
            case R.id.know_button_yes:
                if (checked) {
                    knew_it = 1;
                }
                break;
        }

        updateCard(view, knew_it);
        advanceCard();
    }
}
