# Memory Monitoring Scripts

This directory contains scripts for monitoring and analyzing memory usage of processes.

## Scripts

### memory_analyzer.sh
A comprehensive memory analysis tool that provides detailed memory usage information for a given process.

**Usage:**
```bash
./scripts/memory_analyzer.sh <PID>
```

### memory_monitor.sh
A simple continuous monitoring script that displays memory usage in the terminal every second.

**Usage:**
```bash
./scripts/memory_monitor.sh <PID>
```

**Features:**
- Shows memory usage every second in a table format
- Tracks: RSS, VmSize, VmData, RssAnon, and Thread count
- Color-coded output for easy reading
- Press Ctrl+C to stop monitoring
- Shows total sample count when stopped

## Quick Example

1. **Find your app process:**
   ```bash
   ps aux | grep app
   ```

2. **Start monitoring:**
   ```bash
   ./scripts/memory_monitor.sh 12345
   ```

3. **Sample output:**
   ```
   ================================================================
       Memory Monitor for Process 12345
   ================================================================
   Monitoring memory usage every second...
   Press Ctrl+C to stop

   Time     | RSS(MB) | VmSize(MB) | VmData(MB) | RssAnon(MB) | Threads
   ---------|---------|------------|------------|-------------|--------
   14:30:15 | 184     | 1294       | 569        | 165         | 12
   14:30:16 | 184     | 1294       | 569        | 165         | 12
   14:30:17 | 185     | 1294       | 570        | 166         | 12
   ```

## Memory Metrics

- **RSS**: Physical memory currently used (most important)
- **VmSize**: Total virtual memory allocated
- **VmData**: Data/heap memory
- **RssAnon**: Anonymous pages in physical memory
- **Threads**: Number of active threads
