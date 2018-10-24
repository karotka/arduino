package cz.karotka.aqua

import android.content.Context
import kotlinx.android.synthetic.main.activity_main.*
import android.graphics.Bitmap
import android.os.Build
import android.support.v7.app.AppCompatActivity
import android.support.v7.app.ActionBar
import android.os.Bundle
import android.view.View
import android.webkit.WebSettings
import android.webkit.WebView
import android.webkit.WebViewClient
import android.net.http.SslError
import android.webkit.SslErrorHandler
import android.content.Intent
import android.content.SharedPreferences
import android.net.Proxy.getHost
import android.net.Uri
import android.util.Log
import android.widget.ImageButton
import cz.karotka.aqua.SSLTolerentWebViewClient
import cz.karotka.aqua.ConfigActivity

class MainActivity : AppCompatActivity() {

    private var btn: ImageButton? = null
    private val PREFS_FILENAME = "cz.karotka.aqua.prefs"
    var prefs: SharedPreferences? = null
    val TAG = "MainActivity"

    var proto: String? = null
    var address: String? = null
    var port: String? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        //
        btn = findViewById<View>(R.id.imageButton) as ImageButton
        btn!!.setOnClickListener {
            val intent = Intent(this@MainActivity, ConfigActivity::class.java)
            startActivity(intent)
        }

        //For hiding android actionbar
        val myActionBar = supportActionBar
        myActionBar!!.hide()
    }

    override fun onBackPressed() {
        if (webview.canGoBack()) {
            webview.goBack()
        } else {
            super.onBackPressed()
        }
    }

    override fun onResume() {
        super.onResume()
        // shared preferences
        val prefs = this.getSharedPreferences(PREFS_FILENAME,
                Context.MODE_PRIVATE)

        proto = prefs!!.getString("Proto", "https")
        address = prefs.getString("Address", "10.0.68.112")
        port = prefs.getString("port", "")

        var url = proto + "://" + address
        if (!port.equals("")) {
            url += "" + port
        } else {}
        url += "/index.html"
        open(url)

    }

    public fun open(url: String) {
        // Get the web view settings instance
        val settings = webview.settings

        // Enable java script in web view
        settings.javaScriptEnabled = true

        // Enable and setup web view cache
        settings.setAppCacheEnabled(true)
        settings.cacheMode = WebSettings.LOAD_DEFAULT
        settings.setAppCachePath(cacheDir.path)

        // Enable zooming in web view
        settings.setSupportZoom(true)
        settings.builtInZoomControls = true
        settings.displayZoomControls = true

        // Zoom web view text
        //settings.textZoom = 125

        // Enable disable images in web view
        settings.blockNetworkImage = false
        // Whether the WebView should load image resources
        settings.loadsImagesAutomatically = true

        // More web view settings
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            settings.safeBrowsingEnabled = true  // api 26
        }
        //settings.pluginState = WebSettings.PluginState.ON
        settings.useWideViewPort = true
        settings.loadWithOverviewMode = true
        settings.javaScriptCanOpenWindowsAutomatically = true
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1) {
            settings.mediaPlaybackRequiresUserGesture = false
        }

        // More optional settings, you can enable it by yourself
        settings.domStorageEnabled = true
        settings.setSupportMultipleWindows(true)
        settings.loadWithOverviewMode = true
        settings.allowContentAccess = true
        settings.setGeolocationEnabled(true)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN) {
            settings.allowUniversalAccessFromFileURLs = true
        }

        settings.allowFileAccess = true

        // WebView settings
        webview.fitsSystemWindows = true

        /* if SDK version is greater of 19 then activate hardware acceleration
        otherwise activate software acceleration  */
        webview.setLayerType(View.LAYER_TYPE_HARDWARE, null)

        webview.loadUrl(url)

        // Set web view client
        webview.webViewClient = object : SSLTolerentWebViewClient() {
            override fun onPageStarted(view: WebView, url: String, favicon: Bitmap?) {
                // Page loading started
                // Do something
            }

            override fun onPageFinished(view: WebView, url: String) {
                // Page loading finished
                // Enable disable back forward button
            }
        }
    }
}