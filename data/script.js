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

            li.innerHTML = `
                <span><strong>${timeStr}</strong> (${item.d}s)</span>
                <button class="btn btn-danger" onclick="deleteSchedule(${index})">Eliminar</button>
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

        const payload = {
            h: h,
            m: m,
            d: parseInt(durationVal),
            e: true
        };

        fetch('/api/schedule', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(payload)
        })
            .then(res => res.json())
            .then(() => {
                loadSchedule();
                addForm.reset();
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
