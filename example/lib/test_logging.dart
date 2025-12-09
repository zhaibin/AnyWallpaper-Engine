import 'package:flutter/material.dart';
import 'package:anywp_engine/anywp_engine.dart';

/// Test page for logging level control
/// 
/// Tests the new setLogLevel() and getLogLevel() APIs
class TestLoggingPage extends StatefulWidget {
  const TestLoggingPage({super.key});

  @override
  State<TestLoggingPage> createState() => _TestLoggingPageState();
}

class _TestLoggingPageState extends State<TestLoggingPage> {
  int _currentLogLevel = -1;
  String _logLevelName = 'Unknown';
  final List<String> _logLevelNames = ['Debug', 'Info', 'Warn', 'Error', 'None'];
  
  @override
  void initState() {
    super.initState();
    _loadCurrentLogLevel();
  }
  
  Future<void> _loadCurrentLogLevel() async {
    final level = await AnyWPEngine.getLogLevel();
    setState(() {
      _currentLogLevel = level;
      _logLevelName = level >= 0 && level < _logLevelNames.length 
          ? _logLevelNames[level] 
          : 'Unknown ($level)';
    });
  }
  
  Future<void> _setLogLevel(int level) async {
    final success = await AnyWPEngine.setLogLevel(level);
    if (success) {
      await _loadCurrentLogLevel();
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(
            content: Text('✅ Log level set to: ${_logLevelNames[level]}'),
            duration: const Duration(seconds: 2),
          ),
        );
      }
    } else {
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          const SnackBar(
            content: Text('❌ Failed to set log level (Windows not supported yet)'),
            duration: Duration(seconds: 2),
          ),
        );
      }
    }
  }
  
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Logging Level Control'),
        backgroundColor: Colors.deepPurple,
      ),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            // Current Log Level Display
            Card(
              elevation: 4,
              child: Padding(
                padding: const EdgeInsets.all(16.0),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    const Text(
                      'Current Log Level',
                      style: TextStyle(
                        fontSize: 18,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                    const SizedBox(height: 8),
                    Text(
                      _logLevelName,
                      style: TextStyle(
                        fontSize: 24,
                        fontWeight: FontWeight.bold,
                        color: _getLogLevelColor(_currentLogLevel),
                      ),
                    ),
                    const SizedBox(height: 8),
                    Text(
                      'Level: $_currentLogLevel',
                      style: const TextStyle(color: Colors.grey),
                    ),
                  ],
                ),
              ),
            ),
            
            const SizedBox(height: 24),
            
            // Log Level Buttons
            const Text(
              'Set Log Level:',
              style: TextStyle(
                fontSize: 16,
                fontWeight: FontWeight.bold,
              ),
            ),
            const SizedBox(height: 12),
            
            // Debug
            _buildLogLevelButton(
              level: 0,
              name: '🐛 Debug',
              description: 'All logs (verbose)',
              color: Colors.blue,
            ),
            
            // Info
            _buildLogLevelButton(
              level: 1,
              name: 'ℹ️ Info',
              description: 'General information (default)',
              color: Colors.green,
            ),
            
            // Warn
            _buildLogLevelButton(
              level: 2,
              name: '⚠️ Warn',
              description: 'Warnings only',
              color: Colors.orange,
            ),
            
            // Error
            _buildLogLevelButton(
              level: 3,
              name: '❌ Error',
              description: 'Errors only',
              color: Colors.red,
            ),
            
            // None
            _buildLogLevelButton(
              level: 4,
              name: '🔇 None',
              description: 'Disable all logging',
              color: Colors.grey,
            ),
            
            const Spacer(),
            
            // Platform Support Note
            Container(
              padding: const EdgeInsets.all(12),
              decoration: BoxDecoration(
                color: Colors.yellow.shade100,
                borderRadius: BorderRadius.circular(8),
              ),
              child: const Row(
                children: [
                  Icon(Icons.info_outline, color: Colors.orange),
                  SizedBox(width: 12),
                  Expanded(
                    child: Text(
                      '✅ macOS: Fully supported\n'
                      '⚠️ Windows: Not yet implemented',
                      style: TextStyle(fontSize: 12),
                    ),
                  ),
                ],
              ),
            ),
          ],
        ),
      ),
    );
  }
  
  Widget _buildLogLevelButton({
    required int level,
    required String name,
    required String description,
    required Color color,
  }) {
    final isSelected = level == _currentLogLevel;
    return Padding(
      padding: const EdgeInsets.only(bottom: 8.0),
      child: ElevatedButton(
        onPressed: () => _setLogLevel(level),
        style: ElevatedButton.styleFrom(
          backgroundColor: isSelected ? color : Colors.grey.shade200,
          foregroundColor: isSelected ? Colors.white : Colors.black87,
          minimumSize: const Size(double.infinity, 60),
          shape: RoundedRectangleBorder(
            borderRadius: BorderRadius.circular(8),
            side: BorderSide(
              color: isSelected ? color : Colors.transparent,
              width: 2,
            ),
          ),
        ),
        child: Row(
          children: [
            Text(
              name,
              style: const TextStyle(
                fontSize: 16,
                fontWeight: FontWeight.bold,
              ),
            ),
            const SizedBox(width: 8),
            Expanded(
              child: Text(
                description,
                style: TextStyle(
                  fontSize: 12,
                  color: isSelected ? Colors.white70 : Colors.grey.shade600,
                ),
              ),
            ),
            if (isSelected)
              const Icon(Icons.check_circle, size: 20),
          ],
        ),
      ),
    );
  }
  
  Color _getLogLevelColor(int level) {
    switch (level) {
      case 0: return Colors.blue;
      case 1: return Colors.green;
      case 2: return Colors.orange;
      case 3: return Colors.red;
      case 4: return Colors.grey;
      default: return Colors.black;
    }
  }
}

