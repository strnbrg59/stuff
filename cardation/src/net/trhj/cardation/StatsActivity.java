package net.trhj.cardation;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.EditText;
import java.util.Vector;
import com.jjoe64.graphview.*;
import com.jjoe64.graphview.series.*;

public class StatsActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_stats);
        getActionBar().setTitle("Date-due distribution/"
            + CardDb.getCurrLanguage());

        // Display histogram.
        Vector<ComplexInt> histogram_data = CardDb.duedateHistogram(this,
                                                CardDb.getCurrLanguage());

        GraphView histogram = (GraphView) findViewById(R.id.histogram);
        LineGraphSeries<DataPoint> series = new LineGraphSeries<DataPoint>();
        for (int i=0; i<histogram_data.size(); ++i) {
            ComplexInt item = histogram_data.elementAt(i);
            series.appendData(new DataPoint(item.x_, item.y_),
                                            true,1000);
        }

        GridLabelRenderer grids = histogram.getGridLabelRenderer();
        grids.setHorizontalLabelsVisible(true);
        grids.setHighlightZeroLines(false);
        grids.setHorizontalAxisTitle("log_2(days in future)");

        LegendRenderer legends = histogram.getLegendRenderer();
        legends.setMargin(1);
        legends.setPadding(1);

        //series.setThickness(1);
        histogram.getViewport().setMinX(-1.0);
        histogram.getViewport().setMaxX(histogram_data.elementAt(
            histogram_data.size()-1).x_);

        histogram.addSeries(series);
    }
}
