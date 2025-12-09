import 'dart:io';
import 'package:flutter/foundation.dart';
import 'package:shelf/shelf.dart';
import 'package:shelf/shelf_io.dart' as shelf_io;
import 'package:shelf_static/shelf_static.dart';

/// Local HTTP file server for serving wallpaper HTML files
/// 
/// This server helps avoid CORS issues when loading local files
/// and enables modern Web APIs that require HTTP protocol.
class LocalFileServer {
  HttpServer? _server;
  String _baseUrl = '';
  
  /// Check if server is currently running
  bool get isRunning => _server != null;
  
  /// Get the base URL of the running server
  String get baseUrl => _baseUrl;
  
  /// Start the HTTP server
  /// 
  /// [rootPath] - The root directory to serve files from
  /// Returns the base URL (e.g., http://127.0.0.1:54321)
  Future<String> start(String rootPath) async {
    if (_server != null) {
      debugPrint('[LocalFileServer] Server already running at $_baseUrl');
      return _baseUrl;
    }
    
    try {
      debugPrint('[LocalFileServer] Starting server with root: $rootPath');
      
      // Create static file handler
      final handler = createStaticHandler(
        rootPath,
        defaultDocument: 'index.html',
        listDirectories: true,
      );
      
      // Add CORS headers to allow cross-origin requests
      final corsHandler = Pipeline()
          .addMiddleware(_corsHeaders())
          .addHandler(handler);
      
      // Start server on localhost with random available port
      _server = await shelf_io.serve(
        corsHandler,
        '127.0.0.1',
        0, // Use port 0 to let OS assign an available port
      );
      
      _baseUrl = 'http://${_server!.address.host}:${_server!.port}';
      
      debugPrint('[LocalFileServer] ✅ Server started at $_baseUrl');
      debugPrint('[LocalFileServer]    Root directory: $rootPath');
      debugPrint('[LocalFileServer]    Access examples at: $_baseUrl/examples/');
      
      return _baseUrl;
    } catch (e) {
      debugPrint('[LocalFileServer] ❌ Failed to start server: $e');
      _server = null;
      _baseUrl = '';
      rethrow;
    }
  }
  
  /// Stop the HTTP server
  Future<void> stop() async {
    if (_server == null) {
      debugPrint('[LocalFileServer] Server not running, nothing to stop');
      return;
    }
    
    try {
      debugPrint('[LocalFileServer] Stopping server at $_baseUrl');
      await _server!.close(force: true);
      _server = null;
      _baseUrl = '';
      debugPrint('[LocalFileServer] ✅ Server stopped');
    } catch (e) {
      debugPrint('[LocalFileServer] ❌ Error stopping server: $e');
      _server = null;
      _baseUrl = '';
    }
  }
  
  /// Middleware to add CORS headers
  Middleware _corsHeaders() {
    return (Handler handler) {
      return (Request request) async {
        final response = await handler(request);
        
        return response.change(headers: {
          'Access-Control-Allow-Origin': '*',
          'Access-Control-Allow-Methods': 'GET, POST, PUT, DELETE, OPTIONS',
          'Access-Control-Allow-Headers': 'Origin, Content-Type, Accept',
        });
      };
    };
  }
}

