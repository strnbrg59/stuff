package net.trhj.androidflashcards;

import android.content.Context;
import android.os.Bundle;
import android.graphics.Color;
import android.app.Activity;
import android.view.View;
import android.widget.Toast;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.CheckBox;
import android.util.Log;
import java.util.*;
import java.lang.Math;

public class LearnActivity extends Activity {

    public static class G {
        static CardDb.Card curr_card = null;
        static LinkedList<CardDb.Card> orig_batch = null;
        static LinkedList<CardDb.Card> curr_batch = null;
        static boolean forward = true;
        static int total_rows = 0;
        static boolean clicked_yesnomeh_already = true;
        static int n_left;
        static int max_batch_size;
        static int rand_reversal;
        static NumberPicker cram_picker_ = null;
        static String default_quote_ = "---";
    }

	@Override
    @SuppressWarnings("unchecked")
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_learn);
        setTextSizes();

        G.cram_picker_ = new NumberPicker(0, 8, 0,
            (EditText) findViewById(R.id.cramText));
        CardDb.setCramIncrement(G.cram_picker_.getVal());
        G.total_rows = CardDb.totalRows(getBaseContext());
        G.max_batch_size = ConfigDb.getBatchSize(this);
        G.rand_reversal = ConfigDb.getRandReversal(this);

        G.orig_batch = CardDb.getDueCards(this, G.max_batch_size);
        G.curr_batch = (LinkedList<CardDb.Card>) G.orig_batch.clone();
        G.n_left = G.curr_batch.size();

        advanceCard();
	}

    public static int getMinStreak(Context context) {
        return ConfigDb.getStreak(context);
    }

    public void cramUp(View view)
    {
        G.cram_picker_.up();
        CardDb.setCramIncrement(G.cram_picker_.getVal());
        resample();
        advanceCard();
    }
    public void cramDown(View view)
    {
        G.cram_picker_.down();
        CardDb.setCramIncrement(G.cram_picker_.getVal());
        G.curr_batch.clear();
        resample();
        advanceCard();
    }

    public void setTextSizes() {
        EditText clue = (EditText) findViewById(R.id.clue);
        clue.setTextSize(clue.getTextSize()*1);
        EditText answer = (EditText) findViewById(R.id.answer);
        answer.setTextSize(answer.getTextSize()*1);
    }

    /* We pass rand_reversal_prob, instead of just use G.rand_reversal, cuz
     * sometimes we want to get just the "deterministic" result.
     */
    public boolean shouldGoForward(int importance,
                                   int fwd_streak,
                                   double rand_reversal_prob)
    {
        // Doesn't depend on bkwd_streak, doesn't need to.
        boolean forward = (importance == 0) || (fwd_streak < 0);
        if (!forward) {
            int now = CardationUtils.epochNow();
            Random r = new Random(now);
            int rr = r.nextInt(101);
            Log.e("Cardation", "rr = " + rr + ", rand_reversal_prob = "
                + rand_reversal_prob);
            if (rr < 100*rand_reversal_prob) {
                forward = true;
            }
        }
        return forward;
    }

    /* When cards drop out of G.curr_batch, we replace them with other cards
     * that are now due.
     */
    @SuppressWarnings("unchecked")
    void resample() {
        LinkedList<CardDb.Card> temp_batch =
            CardDb.getDueCards(this, G.max_batch_size);
        Log.w("Cardation", "temp_batch.size() = " + temp_batch.size());

        int batch_size_on_entry = G.curr_batch.size();
    
        while (temp_batch.size() > 0) {
            CardDb.Card temp_card = temp_batch.poll();
            boolean add_it = false;
            boolean duplicate = false;

            // Don't add duplicates.
            for (int i=0; i<G.curr_batch.size(); ++i) {
                if (G.curr_batch.get(i).recto_.equals(temp_card.recto_)) {
                    duplicate = true;
                    break;
                }
            }
            if (duplicate) {
                continue;
            }

            if (G.curr_batch.size() < G.max_batch_size) {
                G.curr_batch.add(temp_card);
            }
        }

        Log.i("Cardation", "G.curr_batch after resample:");
        temp_batch = (LinkedList<CardDb.Card>) G.curr_batch.clone();
        while (temp_batch.size() > 0) {
            CardDb.Card temp_card = temp_batch.poll();
            Log.i("Cardation", temp_card.recto_);
        }            

        G.n_left = G.curr_batch.size();
        if ((batch_size_on_entry == 0) && (G.n_left > 0)) {
            enableWidgets();
        }
    }


    public void advanceCard() {
        EditText info = (EditText) findViewById(R.id.infoEditText);
        info.setEnabled(false);

        Log.w("Cardation", "advanceCard(): G.curr_batch.size() = " +
              G.curr_batch.size());
        if (G.curr_batch.size() == 0) {
            resample();
        }
         
        if (G.curr_batch.size() == 0) {
            // Definitely done now.
            Log.w("Cardation", "Empty batch, no cards are due now.");
            disableWidgets();

            // We're done, but a dummy card is helpful here.
            G.curr_card = new CardDb.Card(" ", " ", 0, 0, 0, 0, "");
        } else {
            G.curr_card = G.curr_batch.poll();
        }

        CheckBox active = (CheckBox) findViewById(R.id.activeCheckBox);
        active.setChecked(G.curr_card.importance_ == 1);

        // Which direction should we go in?
        G.forward = shouldGoForward(G.curr_card.importance_,
                                    G.curr_card.fwd_streak_,
                                    G.rand_reversal/100.0);
                         
        // info box.
        String info_msg = new String("streak = (" +
            Integer.toString(G.curr_card.fwd_streak_) + ", " +
            Integer.toString(G.curr_card.bkwd_streak_) + ")" );
        info_msg += "\n" + G.n_left + " in batch.";
        info_msg += "\n" + CardDb.getNDue() + " now due.";
        info_msg += "\n" + "Total in DB = " + G.total_rows;
        info.setText(info_msg);

        // Populate widgets.
        EditText clue = (EditText) findViewById(R.id.clue);
        EditText answer = (EditText) findViewById(R.id.answer);
        EditText quote = (EditText) findViewById(R.id.quote);

        if (G.forward == true) {
            clue.setText(G.curr_card.recto_);
            answer.setText("?");
        } else {
            answer.setText(G.curr_card.verso_);
            clue.setText("?");
        }
        quote.setText(G.default_quote_);
        if (G.n_left == 0) {
            clue.setText("---");
            answer.setText("---");
            quote.setText(G.default_quote_);
        }

    }

    /** Display the answer and the quote */
    public void showAnswer(View view) {
        assert(G.curr_card != null);
        if (G.forward == true) {
            EditText answer = (EditText) findViewById(R.id.answer);
            answer.setText(G.curr_card.verso_);
        } else {
            EditText clue = (EditText) findViewById(R.id.clue);
            clue.setText(G.curr_card.recto_);
        }
        EditText quote = (EditText) findViewById(R.id.quote);
        quote.setText(G.curr_card.quote_);
    }

    // 
    // Store the updated streaks and due dates.
    // If anything on the card was edited, store that too.
    //
    public void updateCard(View view, int knew_it) {
        CardDb.Card new_card = (CardDb.Card)G.curr_card.clone();

        //
        // Update the streaks.
        //
        if (knew_it == 1) {                 // yes
            if (G.forward == true) {
                new_card.fwd_streak_ += 1;
            } else {
                new_card.bkwd_streak_ += 1;
            }
        } else if (knew_it == -1) {         //no
            if (G.forward == true) {
                new_card.fwd_streak_ =
                    Math.min(-1,
                             Math.max(getMinStreak(this),
                                      new_card.fwd_streak_ - 1));
            } else {
                if (new_card.bkwd_streak_ == getMinStreak(this)) {
                    // Give up on backward for a while.  Prepare the ground to
                    // go forward next time.
                    new_card.fwd_streak_ = new_card.bkwd_streak_
                        = getMinStreak(this);
                } else {
                    new_card.bkwd_streak_ =
                        Math.min(-1,
                                 Math.max(getMinStreak(this),
                                          new_card.bkwd_streak_ - 1));
                }
            }
        } // If knew_it==0 (meh), leave streaks unchanged.

        //
        // Update the widgets.
        //

        // Update recto, verso and quote in case they've been edited.
        EditText recto = (EditText) findViewById(R.id.clue);
        new_card.recto_ = recto.getText().toString();
        if (new_card.recto_.equals("?")) {
            new_card.recto_ = G.curr_card.recto_;
        }
        EditText verso = (EditText) findViewById(R.id.answer);
        new_card.verso_ = verso.getText().toString();
        if (new_card.verso_.equals("?")) {
            new_card.verso_ = G.curr_card.verso_;
        }
        EditText quote = (EditText) findViewById(R.id.quote);
        new_card.quote_ = quote.getText().toString();
        if (new_card.quote_.equals(G.default_quote_)) {
            new_card.quote_ = G.curr_card.quote_;
        }

        // Delete the card if so indicated.
        if (new_card.recto_.equals("")) {
            CardDb.deleteCard(this, G.curr_card);
            Toast.makeText(this, "Deleted " + G.curr_card.recto_,
                           Toast.LENGTH_LONG).show();
            return;
        }

        // Importance/active checkbox.
        if (((CheckBox) findViewById(R.id.activeCheckBox)).isChecked()) {
            new_card.importance_ = 1;
        } else {
            new_card.importance_ = 0;
        }

        final int max_delay = 365*24*3600;
        int streak;
        boolean new_forward = shouldGoForward(new_card.importance_,
                                              new_card.fwd_streak_, 0.0);
        if (new_forward == true) {
            streak = new_card.fwd_streak_;
        } else {
            streak = new_card.bkwd_streak_;
        }

        // The delay is 0 if streak<0, otherwise it's one day times 2^streak.
        // We add some randomness too, to ensure that a whole bunch of cards
        // don't all come due at the same time.
        int now = CardationUtils.epochNow();
        Random r = new Random(now);
        double day_secs = 24*3600;
        int delay;
        if (streak < 0) {
            delay = 0;
        } else {
            // XXX As we approach 2036, everything will bunch up against the
            // maximal Unix epoch.
            double ideal_delay = day_secs *
                Math.pow(2, streak) * (1.0 + (r.nextInt(101) - 50)/1000.0);
            int max_int = (int)(Math.pow(2, 31) - 1);
            delay = (int)
                Math.min(ideal_delay, max_int - now);
        }

        int due = now + Math.min(delay, max_delay);
        // We never needed two due-date columns, but let's not monkey with the
        // DB's schema now or we'll need to do a custom restore.
        new_card.due_ = due;
        Log.w("Cardation", "due in " +
            (due - now)/day_secs + " days from now.");

        if (G.curr_card.recto_.equals(new_card.recto_)) {
            CardDb.updateByRecto(getBaseContext(), new_card);
        } else {
            CardDb.deleteCard(getBaseContext(), G.curr_card);
            CardDb.saveCard(getBaseContext(), new_card);
            Toast.makeText(this, "Modified clue, ok", Toast.LENGTH_LONG).show();
            Log.w("Cardation", "Changed recto from " + G.curr_card.recto_+" to "
                               + new_card.recto_);
        }

        if (due > now) {
            resample();
        }

        Log.w("Cardation", "---------------------------------");
    }


    void disableWidgets() {
        EditText clue = (EditText) findViewById(R.id.clue);
        clue.setEnabled(false);
        EditText answer = (EditText) findViewById(R.id.answer);
        answer.setEnabled(false);
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
        EditText clue = (EditText) findViewById(R.id.clue);
        clue.setEnabled(true);
        EditText answer = (EditText) findViewById(R.id.answer);
        answer.setEnabled(true);
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
        // all that happens is that the answer and the clue are displayed.
        G.clicked_yesnomeh_already ^= true;
        if (! G.clicked_yesnomeh_already) {
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
