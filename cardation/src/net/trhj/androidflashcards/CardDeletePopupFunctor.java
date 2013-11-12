package net.trhj.androidflashcards;

import android.app.Activity;
import android.util.Log;

public class CardDeletePopupFunctor implements PopupFunctor {
    String recto_;

    CardDeletePopupFunctor(String recto) {
        super();
        recto_ = recto;
    }

    public void doit(Activity activity, Object extra) {
        CardDb.Card card = CardDb.findCard(activity, recto_);
        CardDb.deleteCard(activity, card);
        Log.w("Cardation", "CardDeletePopupFunctor.doit()");
    }
}
