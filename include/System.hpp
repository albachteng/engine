class System {
public:
  virtual ~System() = default;
  virtual void update(float deltaTime) = 0;
  virtual void init() = 0;
  virtual void shutdown() = 0;
};
