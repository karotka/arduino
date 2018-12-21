package cz.karotka.aqua

import android.net.http.SslError
import android.webkit.SslErrorHandler
import android.webkit.WebView
import android.webkit.WebViewClient

// SSL Error Tolerant Web View Client
open class SSLTolerentWebViewClient : WebViewClient() {

    override fun onReceivedSslError(view: WebView, handler: SslErrorHandler, error: SslError) {
        handler.proceed() // Ignore SSL certificate errors
    }

}