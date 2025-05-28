import tkinter as tk
from tkinter import ttk
import time
from datetime import datetime
from enum import Enum
import threading
import math

# Streaking pattern options for users
class StreakingPattern(Enum):
    SIMPLE_LINE = "Simple Line"
    THREE_SECTOR = "Three Sector" 
    QUADRANT = "Quadrant"
    SPIRAL = "Spiral"
    ZIGZAG = "Zigzag"

# System status for users (simplified)
class SystemStatus(Enum):
    READY = "Ready"
    PROCESSING = "Processing Dish"
    COLLECTING = "Collecting Sample"
    STREAKING = "Streaking in Progress"
    FINISHING = "Completing Dish"
    ERROR = "Error - Check System"

class LoadingScreen:
    def __init__(self, parent, callback):
        self.parent = parent
        self.callback = callback
        self.canvas = None
        self.create_loading_screen()
        
    def create_loading_screen(self):
        # Create loading frame
        self.loading_frame = tk.Frame(self.parent, bg='#ffffff')
        self.loading_frame.pack(fill='both', expand=True)
        
        # Create canvas for hexagonal background
        self.canvas = tk.Canvas(self.loading_frame, bg='#ffffff', highlightthickness=0)
        self.canvas.pack(fill='both', expand=True)
        
        # Draw hexagonal pattern in blue
        self.draw_hexagonal_pattern()
        
        # Add InoQ branding
        self.canvas.create_text(700, 350, text="InoQ", 
                               font=('Segoe UI', 52, 'bold'),
                               fill='#1e3a8a', anchor='center')
        
        self.canvas.create_text(700, 410, text="Automated Petri Dish Streaking System",
                               font=('Segoe UI', 16),
                               fill='#666666', anchor='center')
        
        # Loading animation
        self.loading_text = self.canvas.create_text(700, 500, text="Starting up...",
                                                   font=('Segoe UI', 14),
                                                   fill='#888888', anchor='center')
        
        # Start loading animation
        self.animate_loading()
        
        # Auto-complete loading after 3 seconds
        self.parent.after(3000, self.complete_loading)
    
    def draw_hexagonal_pattern(self):
        # Create subtle hexagonal pattern in blue theme
        width = 1400
        height = 900
        
        hex_size = 35
        hex_spacing = 60
        
        for x in range(-100, width + 100, hex_spacing):
            for y in range(-100, height + 100, hex_spacing):
                # Offset every other row
                offset_x = (hex_spacing // 2) if (y // hex_spacing) % 2 else 0
                self.draw_hexagon(x + offset_x, y, hex_size, '#e8f2ff', 1)
    
    def draw_hexagon(self, x, y, size, color, width):
        points = []
        for i in range(6):
            angle = math.pi * i / 3
            px = x + size * math.cos(angle)
            py = y + size * math.sin(angle)
            points.extend([px, py])
        
        self.canvas.create_polygon(points, outline=color, fill='', width=width)
    
    def animate_loading(self):
        texts = ["Starting up...", "Initializing hardware...", "System ready!"]
        current_text = getattr(self, 'loading_step', 0)
        
        if current_text < len(texts):
            self.canvas.itemconfig(self.loading_text, text=texts[current_text])
            self.loading_step = current_text + 1
            self.parent.after(1000, self.animate_loading)
    
    def complete_loading(self):
        self.loading_frame.destroy()
        self.callback()

class InoQControlPanel:
    def __init__(self, root):
        self.root = root
        self.root.title("InoQ - Automated Petri Dish Streaker")
        self.root.geometry("1400x900")
        self.root.configure(bg='#f8fafc')
        
        # System state (user-friendly)
        self.current_status = SystemStatus.READY
        self.is_running = False
        self.dishes_in_system = 5  # Simulated
        self.current_dish = 0
        self.selected_pattern = StreakingPattern.SIMPLE_LINE
        self.start_time = None
        self.dishes_completed = 0
        self.estimated_time = 0
        
        # Clean, professional color scheme
        self.colors = {
            'bg_primary': '#f8fafc',
            'bg_secondary': '#ffffff', 
            'bg_card': '#ffffff',
            'border': '#e2e8f0',
            'accent_primary': '#1e3a8a',     # Navy blue
            'accent_secondary': '#3b82f6',   # Lighter blue
            'accent_light': '#dbeafe',       # Very light blue
            'text_primary': '#1e293b',
            'text_secondary': '#64748b',
            'text_muted': '#94a3b8',
            'success': '#059669',
            'warning': '#d97706',
            'error': '#dc2626',
            'idle': '#6b7280'
        }
        
        # Show loading screen first
        LoadingScreen(self.root, self.initialize_main_interface)
        
    def initialize_main_interface(self):
        self.create_widgets()
        self.update_display()
        
    def create_widgets(self):
        # Main container with light background
        main_frame = tk.Frame(self.root, bg=self.colors['bg_primary'])
        main_frame.pack(fill='both', expand=True, padx=40, pady=30)
        
        # Header
        self.create_header(main_frame)
        
        # Main content area
        content_frame = tk.Frame(main_frame, bg=self.colors['bg_primary'])
        content_frame.pack(fill='both', expand=True, pady=(30, 0))
        
        # Left side - Status and Visual
        left_panel = tk.Frame(content_frame, bg=self.colors['bg_primary'])
        left_panel.pack(side='left', fill='both', expand=True, padx=(0, 20))
        
        self.create_status_display(left_panel)
        self.create_system_visual(left_panel)
        
        # Right side - Controls and Settings
        right_panel = tk.Frame(content_frame, bg=self.colors['bg_primary'], width=400)
        right_panel.pack(side='right', fill='y', padx=(20, 0))
        right_panel.pack_propagate(False)
        
        self.create_main_controls(right_panel)
        self.create_settings_panel(right_panel)
        self.create_dish_info(right_panel)

    def create_header(self, parent):
        header_frame = tk.Frame(parent, bg=self.colors['bg_primary'])
        header_frame.pack(fill='x', pady=(0, 30))
        
        # Logo and title
        title_frame = tk.Frame(header_frame, bg=self.colors['bg_primary'])
        title_frame.pack(side='left')
        
        title_label = tk.Label(title_frame, text="InoQ", 
                              bg=self.colors['bg_primary'],
                              fg=self.colors['accent_primary'],
                              font=('Segoe UI', 36, 'bold'))
        title_label.pack(anchor='w')
        
        subtitle_label = tk.Label(title_frame, text="Automated Petri Dish Streaking",
                                 bg=self.colors['bg_primary'],
                                 fg=self.colors['text_secondary'],
                                 font=('Segoe UI', 16))
        subtitle_label.pack(anchor='w')
        
        # System status indicator (top right)
        status_frame = tk.Frame(header_frame, bg=self.colors['bg_primary'])
        status_frame.pack(side='right')
        
        self.system_status_label = tk.Label(status_frame, text="● SYSTEM READY",
                                           bg=self.colors['bg_primary'],
                                           fg=self.colors['success'],
                                           font=('Segoe UI', 14, 'bold'))
        self.system_status_label.pack()

    def create_card_frame(self, parent, title=None, height=None):
        # Create modern card with subtle shadow effect
        card_container = tk.Frame(parent, bg=self.colors['bg_primary'])
        card_container.pack(fill='x', pady=(0, 25))
        
        if height:
            card_container.configure(height=height)
            card_container.pack_propagate(False)
        
        # Shadow effect (subtle)
        shadow = tk.Frame(card_container, bg='#e2e8f0', height=2)
        shadow.pack(side='bottom', fill='x')
        
        card = tk.Frame(card_container, bg=self.colors['bg_card'], 
                       relief='solid', bd=1, highlightbackground=self.colors['border'])
        card.pack(fill='both', expand=True)
        
        # Card header if needed
        if title:
            header = tk.Frame(card, bg=self.colors['accent_light'], height=50)
            header.pack(fill='x')
            header.pack_propagate(False)
            
            title_label = tk.Label(header, text=title,
                                  bg=self.colors['accent_light'],
                                  fg=self.colors['accent_primary'],
                                  font=('Segoe UI', 14, 'bold'))
            title_label.pack(side='left', padx=25, pady=15)
        
        # Card content
        content = tk.Frame(card, bg=self.colors['bg_card'])
        content.pack(fill='both', expand=True, padx=30, pady=25)
        
        return content

    def create_status_display(self, parent):
        content = self.create_card_frame(parent, "Current Operation")
        
        # Large status display
        self.status_label = tk.Label(content, text="Ready to Start",
                                    bg=self.colors['bg_card'],
                                    fg=self.colors['accent_primary'],
                                    font=('Segoe UI', 32, 'bold'))
        self.status_label.pack(anchor='w', pady=(0, 15))
        
        # Progress bar
        self.progress_frame = tk.Frame(content, bg=self.colors['bg_card'])
        self.progress_frame.pack(fill='x', pady=(0, 20))
        
        self.progress_bar = tk.Canvas(self.progress_frame, height=8, bg=self.colors['border'], 
                                     highlightthickness=0)
        self.progress_bar.pack(fill='x')
        
        # Time and progress info
        info_frame = tk.Frame(content, bg=self.colors['bg_card'])
        info_frame.pack(fill='x')
        
        self.time_label = tk.Label(info_frame, text="",
                                  bg=self.colors['bg_card'],
                                  fg=self.colors['text_secondary'],
                                  font=('Segoe UI', 12))
        self.time_label.pack(side='left')
        
        self.progress_text = tk.Label(info_frame, text="",
                                     bg=self.colors['bg_card'],
                                     fg=self.colors['text_secondary'],
                                     font=('Segoe UI', 12))
        self.progress_text.pack(side='right')

    def create_system_visual(self, parent):
        content = self.create_card_frame(parent, "System Overview", height=300)
        
        # Visual representation of the system
        visual_canvas = tk.Canvas(content, bg=self.colors['bg_card'], 
                                 highlightthickness=0, height=200)
        visual_canvas.pack(fill='both', expand=True, pady=20)
        
        # Draw simple system representation
        self.draw_system_diagram(visual_canvas)
        
        # Store canvas for updates
        self.visual_canvas = visual_canvas

    def draw_system_diagram(self, canvas):
        # Simple visual representation of the system
        width = canvas.winfo_reqwidth() or 600
        height = 200
        
        # Platform circle
        platform_x, platform_y = width//2, height//2 + 20
        canvas.create_oval(platform_x - 60, platform_y - 60, 
                          platform_x + 60, platform_y + 60,
                          outline=self.colors['accent_primary'], width=3, fill=self.colors['accent_light'])
        canvas.create_text(platform_x, platform_y, text="Platform", 
                          font=('Segoe UI', 10, 'bold'), fill=self.colors['accent_primary'])
        
        # Polar arm
        arm_x = platform_x - 80
        canvas.create_line(arm_x, platform_y, arm_x - 40, platform_y - 40, 
                          width=4, fill=self.colors['accent_secondary'])
        canvas.create_oval(arm_x - 45, platform_y - 45, arm_x - 35, platform_y - 35,
                          fill=self.colors['accent_secondary'])
        
        # Sample vial
        vial_x = width - 100
        canvas.create_rectangle(vial_x, platform_y - 30, vial_x + 20, platform_y + 30,
                               fill=self.colors['warning'], outline=self.colors['warning'])
        canvas.create_text(vial_x + 10, platform_y + 45, text="Sample", 
                          font=('Segoe UI', 9), fill=self.colors['text_secondary'])
        
        # Dish stack
        stack_x = 80
        for i in range(3):
            canvas.create_oval(stack_x - 25, platform_y + 20 - i*5, 
                              stack_x + 25, platform_y + 40 - i*5,
                              outline=self.colors['idle'], fill='white')
        canvas.create_text(stack_x, platform_y + 60, text="Dishes", 
                          font=('Segoe UI', 9), fill=self.colors['text_secondary'])

    def create_main_controls(self, parent):
        content = self.create_card_frame(parent, "Controls")
        
        # Large start button
        self.start_button = tk.Button(content, text="START PROCESSING",
                                     command=self.start_processing,
                                     bg=self.colors['success'],
                                     fg='white',
                                     font=('Segoe UI', 16, 'bold'),
                                     relief='flat',
                                     cursor='hand2',
                                     pady=20,
                                     width=20)
        self.start_button.pack(fill='x', pady=(0, 15))
        
        # Control buttons row
        button_frame = tk.Frame(content, bg=self.colors['bg_card'])
        button_frame.pack(fill='x', pady=(0, 15))
        
        self.stop_button = tk.Button(button_frame, text="STOP",
                                    command=self.stop_processing,
                                    bg=self.colors['error'],
                                    fg='white',
                                    font=('Segoe UI', 12, 'bold'),
                                    relief='flat',
                                    cursor='hand2',
                                    pady=12,
                                    state='disabled')
        self.stop_button.pack(side='left', fill='x', expand=True, padx=(0, 8))
        
        self.restart_button = tk.Button(button_frame, text="RESTART",
                                       command=self.restart_system,
                                       bg=self.colors['warning'],
                                       fg='white',
                                       font=('Segoe UI', 12, 'bold'),
                                       relief='flat',
                                       cursor='hand2',
                                       pady=12)
        self.restart_button.pack(side='left', fill='x', expand=True, padx=(8, 0))
        
        # Emergency stop (prominent)
        self.emergency_button = tk.Button(content, text="🛑 EMERGENCY STOP",
                                         command=self.emergency_stop,
                                         bg='#b91c1c',
                                         fg='white',
                                         font=('Segoe UI', 14, 'bold'),
                                         relief='flat',
                                         cursor='hand2',
                                         pady=15)
        self.emergency_button.pack(fill='x')

    def create_settings_panel(self, parent):
        content = self.create_card_frame(parent, "Streaking Settings")
        
        # Pattern selection
        pattern_label = tk.Label(content, text="Streaking Pattern:",
                                bg=self.colors['bg_card'],
                                fg=self.colors['text_primary'],
                                font=('Segoe UI', 12, 'bold'))
        pattern_label.pack(anchor='w', pady=(0, 8))
        
        self.pattern_var = tk.StringVar(value=self.selected_pattern.value)
        pattern_menu = ttk.Combobox(content, textvariable=self.pattern_var,
                                   values=[p.value for p in StreakingPattern],
                                   state='readonly', font=('Segoe UI', 11))
        pattern_menu.pack(fill='x', pady=(0, 20))
        pattern_menu.bind('<<ComboboxSelected>>', self.on_pattern_change)
        
        # Sample volume
        volume_label = tk.Label(content, text="Sample Volume (μL):",
                               bg=self.colors['bg_card'],
                               fg=self.colors['text_primary'],
                               font=('Segoe UI', 12, 'bold'))
        volume_label.pack(anchor='w', pady=(0, 8))
        
        self.volume_var = tk.StringVar(value="10")
        volume_entry = tk.Entry(content, textvariable=self.volume_var,
                               font=('Segoe UI', 11), justify='center')
        volume_entry.pack(fill='x', pady=(0, 20))
        
        # Speed setting
        speed_label = tk.Label(content, text="Streaking Speed:",
                              bg=self.colors['bg_card'],
                              fg=self.colors['text_primary'],
                              font=('Segoe UI', 12, 'bold'))
        speed_label.pack(anchor='w', pady=(0, 8))
        
        self.speed_var = tk.StringVar(value="Normal")
        speed_menu = ttk.Combobox(content, textvariable=self.speed_var,
                                 values=["Slow", "Normal", "Fast"],
                                 state='readonly', font=('Segoe UI', 11))
        speed_menu.pack(fill='x')

    def create_dish_info(self, parent):
        content = self.create_card_frame(parent, "Dish Information")
        
        # Dishes in system
        self.dishes_info = tk.Label(content, text=f"Dishes Loaded: {self.dishes_in_system}",
                                   bg=self.colors['bg_card'],
                                   fg=self.colors['text_primary'],
                                   font=('Segoe UI', 14, 'bold'))
        self.dishes_info.pack(anchor='w', pady=(0, 10))
        
        # Current progress
        self.current_progress = tk.Label(content, text="Completed: 0",
                                        bg=self.colors['bg_card'],
                                        fg=self.colors['text_secondary'],
                                        font=('Segoe UI', 12))
        self.current_progress.pack(anchor='w', pady=(0, 10))
        
        # Estimated time
        self.time_estimate = tk.Label(content, text="Estimated time: --:--",
                                     bg=self.colors['bg_card'],
                                     fg=self.colors['text_secondary'],
                                     font=('Segoe UI', 12))
        self.time_estimate.pack(anchor='w')

    def start_processing(self):
        self.is_running = True
        self.start_time = time.time()
        self.current_status = SystemStatus.PROCESSING
        self.estimated_time = self.dishes_in_system * 120  # 2 minutes per dish
        
        self.start_button.config(state='disabled')
        self.stop_button.config(state='normal')
        
        # Start processing simulation
        threading.Thread(target=self.simulate_processing, daemon=True).start()

    def stop_processing(self):
        self.is_running = False
        self.current_status = SystemStatus.READY
        
        self.start_button.config(state='normal')
        self.stop_button.config(state='disabled')

    def restart_system(self):
        self.stop_processing()
        self.dishes_completed = 0
        self.current_dish = 0
        # Simulate system restart
        self.current_status = SystemStatus.READY

    def emergency_stop(self):
        self.stop_processing()
        self.current_status = SystemStatus.ERROR
        # Could add emergency procedures here

    def on_pattern_change(self, event):
        pattern_name = self.pattern_var.get()
        self.selected_pattern = StreakingPattern(pattern_name)

    def simulate_processing(self):
        """Simulate processing dishes"""
        statuses = [SystemStatus.COLLECTING, SystemStatus.STREAKING, SystemStatus.FINISHING]
        
        for dish in range(self.dishes_in_system):
            if not self.is_running:
                break
                
            self.current_dish = dish + 1
            
            for i, status in enumerate(statuses):
                if not self.is_running:
                    break
                    
                self.current_status = status
                self.root.after(0, self.update_display)
                time.sleep(2 + i)  # Different times for different operations
            
            if self.is_running:
                self.dishes_completed += 1
        
        if self.is_running:
            self.stop_processing()

    def update_display(self):
        # Update status display
        status_texts = {
            SystemStatus.READY: "Ready to Start",
            SystemStatus.PROCESSING: f"Processing Dish {self.current_dish}",
            SystemStatus.COLLECTING: f"Collecting Sample - Dish {self.current_dish}",
            SystemStatus.STREAKING: f"Streaking Pattern - Dish {self.current_dish}",
            SystemStatus.FINISHING: f"Finishing Dish {self.current_dish}",
            SystemStatus.ERROR: "System Error - Check Hardware"
        }
        
        self.status_label.config(text=status_texts.get(self.current_status, "Unknown"))
        
        # Update system status indicator
        if self.current_status == SystemStatus.ERROR:
            self.system_status_label.config(text="● SYSTEM ERROR", fg=self.colors['error'])
        elif self.is_running:
            self.system_status_label.config(text="● PROCESSING", fg=self.colors['warning'])
        else:
            self.system_status_label.config(text="● SYSTEM READY", fg=self.colors['success'])
        
        # Update progress bar
        if self.is_running and self.dishes_in_system > 0:
            progress = self.dishes_completed / self.dishes_in_system
            self.update_progress_bar(progress)
            
            # Update time info
            if self.start_time:
                elapsed = time.time() - self.start_time
                remaining = max(0, self.estimated_time - elapsed)
                
                elapsed_str = f"{int(elapsed//60):02d}:{int(elapsed%60):02d}"
                remaining_str = f"{int(remaining//60):02d}:{int(remaining%60):02d}"
                
                self.time_label.config(text=f"Elapsed: {elapsed_str}")
                self.progress_text.config(text=f"Remaining: {remaining_str}")
        else:
            self.update_progress_bar(0)
            self.time_label.config(text="")
            self.progress_text.config(text="")
        
        # Update dish info
        self.current_progress.config(text=f"Completed: {self.dishes_completed}")
        if self.estimated_time > 0 and not self.is_running:
            self.time_estimate.config(text=f"Estimated time: {self.estimated_time//60:.0f} min")
        
        # Schedule next update
        self.root.after(200, self.update_display)

    def update_progress_bar(self, progress):
        self.progress_bar.delete("all")
        width = self.progress_bar.winfo_width()
        if width > 1:
            # Background
            self.progress_bar.create_rectangle(0, 0, width, 8, 
                                             fill=self.colors['border'], outline="")
            # Progress
            if progress > 0:
                self.progress_bar.create_rectangle(0, 0, width * progress, 8,
                                                 fill=self.colors['accent_primary'], outline="")

def main():
    root = tk.Tk()
    app = InoQControlPanel(root)
    root.mainloop()

if __name__ == "__main__":
    main()