package net.trhj.cardation;

import android.app.Activity;

public interface PopupFunctor {
    public void onPositiveButton(Activity activity, Object extra);
    public void onNegativeButton(Activity activity);
}
