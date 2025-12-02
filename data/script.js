document.addEventListener('DOMContentLoaded', () => {
    const clockEl = document.getElementById('clock');
    const scheduleListEl = document.getElementById('schedule-list');
    const addForm = document.getElementById('add-form');
    const btnRing = document.getElementById('btn-ring');

    // Update Clock and Status
    function updateStatus() {
        fetch('/api/status')
            .then(res => res.json())
            .then(data => {
                clockEl.textContent = data.time;
            })
            .catch(err => console.error('Error fetching status:', err));
    }

    // Load Schedule
    function loadSchedule() {
        fetch('/api/schedule')
            .then(res => res.json())
            .then(data => {
                renderSchedule(data);
            })
            .catch(err => console.error('Error loading schedule:', err));
    }

    function renderSchedule(schedules) {
        scheduleListEl.innerHTML = '';
        schedules.forEach((item, index) => {
            const li = document.createElement('li');
            const timeStr = `${String(item.h).padStart(2, '0')}:${String(item.m).padStart(2, '0')}`;

            // Decode days bitmask
            let daysStr = "";
            const daysMap = ["Dom", "Lun", "Mar", "Mié", "Jue", "Vie", "Sáb"];
            let activeDays = [];

            // Default to all days if days property is missing (backward compatibility)
            let mask = item.days !== undefined ? item.days : 127;

            for (let i = 0; i < 7; i++) {
                if ((mask >> i) & 1) {
                    activeDays.push(daysMap[i]);
                }
            }

            if (activeDays.length === 7) daysStr = "Todos los días";
            else if (activeDays.length === 5 && mask === 62) daysStr = "Lun-Vie"; // 00111110
            else daysStr = activeDays.join(", ");

            li.innerHTML = `
                <div class="d-flex justify-content-between align-items-center w-100">
                    <div>
                        <strong>${timeStr}</strong> <small class="text-muted">(${item.d}s)</small><br>
                        <small class="text-primary">${daysStr}</small>
                    </div>
                    <button class="btn btn-danger btn-sm" onclick="deleteSchedule(${index})">X</button>
                </div>
            `;
            scheduleListEl.appendChild(li);
        });
    }

    // Add Schedule
    addForm.addEventListener('submit', (e) => {
        e.preventDefault();
        const timeVal = document.getElementById('time').value;
        const durationVal = document.getElementById('duration').value;

        const [h, m] = timeVal.split(':').map(Number);

        // Calculate Days Bitmask
        let daysMask = 0;
        // Checkboxes: day0 (Sun) to day6 (Sat). Note: My HTML used value=1,2,4... for Mon-Fri.
        // Let's correct the logic to match standard: Sun=0, Mon=1...Sat=6

        // Sun (Bit 0)
        if (document.getElementById('day0').checked) daysMask |= (1 << 0);
        // Mon (Bit 1)
        if (document.getElementById('day1').checked) daysMask |= (1 << 1);
        // Tue (Bit 2)
        if (document.getElementById('day2').checked) daysMask |= (1 << 2);
        // Wed (Bit 3)
        if (document.getElementById('day3').checked) daysMask |= (1 << 3);
        // Thu (Bit 4)
        if (document.getElementById('day4').checked) daysMask |= (1 << 4);
        // Fri (Bit 5)
        if (document.getElementById('day5').checked) daysMask |= (1 << 5);
        // Sat (Bit 6)
        if (document.getElementById('day6').checked) daysMask |= (1 << 6);

        const payload = {
            h: h,
            m: m,
            d: parseInt(durationVal),
            e: true,
            days: daysMask
        };

        fetch('/api/schedule', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(payload)
        })
            .then(res => res.json())
            .then(() => {
                loadSchedule();
                // Don't reset form completely to keep duration/days settings if desired, or just reset time
                // addForm.reset(); 
            });
    });

    // Manual Trigger
    btnRing.addEventListener('click', () => {
        fetch('/api/trigger', { method: 'POST' })
            .then(res => res.json())
            .then(data => console.log('Triggered:', data));
    });

    // Delete Schedule (Global function for onclick)
    window.deleteSchedule = function (index) {
        if (!confirm('¿Estás seguro de eliminar este horario?')) return;

        fetch('/api/remove_schedule', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ index: index })
        })
            .then(res => res.json())
            .then(() => loadSchedule());
    };

    // Initial Load
    updateStatus();
    loadSchedule();

    // Polling
    setInterval(updateStatus, 1000);
});
