package net.trhj.cardation;

import android.util.Log;
import android.widget.Toast;
import java.io.*;
import java.util.*;
import com.jjoe64.graphview.*;
import com.jjoe64.graphview.series.*;

public class Histogram
{
    private GraphView widget_;
    ArrayList<Integer> duedates_;

    public Histogram(GraphView w, ArrayList<Integer> dd)
    {
        widget_ = w;
        duedates_ = new ArrayList<Integer>();
        resetDuedates(dd);
    }

    public void setTitle(String title)
    {
        widget_.setTitle(title);
    }

    public void resetDuedates(ArrayList<Integer> dd)
    {
        duedates_.clear();
        duedates_.addAll(dd);
        Collections.sort(duedates_);
    }

    public void clear()
    {
        widget_.removeAllSeries();
    }

    public void show()
    {
        clear();

        Vector<ComplexInt> bin_counts = bin();

        // Display histogram.
        BarGraphSeries<DataPoint> series = new BarGraphSeries<DataPoint>();
        Boolean all_zero = true;
        int i = 0;
        int total = 0;
        for (; i<bin_counts.size(); ++i) {
            ComplexInt item = bin_counts.elementAt(i);
            series.appendData(new DataPoint(item.x_, item.y_),
                                            true,1000);
            total += item.y_;
            if (item.y_ != 0) {
                all_zero = false;
            }
        }

        series.setSpacing(75); // 50 doesn't do anything on my device
        series.setDrawValuesOnTop(true); // Doesn't have any effect

        GridLabelRenderer grids = widget_.getGridLabelRenderer();
        grids.setHorizontalLabelsVisible(true);
        grids.setHighlightZeroLines(false);
        grids.setHorizontalAxisTitle("log_2(days in future)");
        grids.setVerticalAxisTitle("Out of " + total + " total words");

        LegendRenderer legends = widget_.getLegendRenderer();
        legends.setMargin(1);
        legends.setPadding(1);

        //series.setThickness(1);
        widget_.getViewport().setYAxisBoundsManual(all_zero);
        if (all_zero) {
            widget_.getViewport().setMaxY(1.0);
            widget_.getViewport().setMinY(0.0);
        }

        widget_.addSeries(series);
    }

    Vector<ComplexInt> bin()
    {
        // Make histogram with power-of-two time categories.

        Log.i("Cardation", "duedates_.size() = " + duedates_.size());
        final int now = CardationUtils.epochNow();
        int later_than = 0;
        int earlier_than = now;
        final int day_seconds = 24*3600;
        final int n_bins = 12;
        Vector<ComplexInt> result = new Vector<ComplexInt>();
        int y = 0;

        for (int x=0; x<=n_bins-2; ++x) {
            int count = 0;
            while (    (y < duedates_.size())
                    && (duedates_.get(y) >= later_than)
                    && (duedates_.get(y) <  earlier_than)) {
                ++count;
                ++y;
            }

            result.add(new ComplexInt(x-1, count));

            later_than = earlier_than;
            if (x == n_bins-2) {
                earlier_than = Integer.MAX_VALUE;
            } else {
                earlier_than += (int)(Math.pow(2,x))*day_seconds;
            }
        }

        return result;
    }
}
