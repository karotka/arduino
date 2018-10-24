package cz.karotka.aqua;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.view.Menu;
import android.view.View;
import android.widget.TextView;

public class ConfigActivity extends Activity {

    SharedPreferences sharedpreferences;
    TextView proto;
    TextView address;
    TextView port;

    public static final String PREFS_FILENAME = "cz.karotka.aqua.prefs";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_config);

        proto = (TextView) findViewById(R.id.proto);
        address = (TextView) findViewById(R.id.address);
        port = (TextView) findViewById(R.id.port);

        sharedpreferences = getSharedPreferences(PREFS_FILENAME,
                Context.MODE_PRIVATE);

        if (sharedpreferences.contains("Proto")) {
            proto.setText(sharedpreferences.getString("Proto", ""));
        }
        if (sharedpreferences.contains("Address")) {
            address.setText(sharedpreferences.getString("Address", ""));
        }
        if (sharedpreferences.contains("Port")) {
            port.setText(sharedpreferences.getString("Port", ""));
        }
    }

    public void Save(View view) {
        String p = proto.getText().toString();
        String a = address.getText().toString();
        String o = port.getText().toString();
        SharedPreferences.Editor editor = sharedpreferences.edit();

        editor.putString("Proto", p);
        editor.putString("Address", a);
        editor.putString("Port", o);
        editor.commit();
        finish();
    }

}
