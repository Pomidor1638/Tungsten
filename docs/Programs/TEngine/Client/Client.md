
## Purpose
Клиентская часть игры. Управляет окном, вводом, рендером, звуком, клиентским состоянием и обменом данными с сервером.

## Responsibility
- Создаёт и уничтожает клиентские модули.
- Выполняет главный клиентский цикл.
- Обрабатывает SDL-события.
- Читает состояние Input.
- Формирует пользовательские команды.
- Получает snapshots от сервера.
- Обновляет клиентское представление игрового мира.
- Передаёт данные в Renderer и Audio.

## Owns
- Window
- Input
- Renderer
- Audio
- Client-side GameState
- Client state
- На этапе прототипа: LocalServer

## Inputs
- SDL events
- Keyboard/mouse/gamepad input
- Snapshots from server
- Map selection requests
- Timing data

## Outputs
- UserCmd to server
- Render frame to Renderer
- Audio commands to Audio
- Client state transitions

## Does not
- Не является авторитетом игровой логики.
- Не изменяет ServerWorld напрямую.
- Не хранит OpenGL-ресурсы вне Renderer.
- Не обрабатывает физику серверного мира.
- Не решает урон, смерть, правила игры.
- В финальной архитектуре не должен владеть Server.

## Notes
На этапе прототипа Client временно владеет LocalServer.
Позже LocalServer должен быть вынесен в Application/Session, а Client должен общаться с сервером через Transport/Connection.