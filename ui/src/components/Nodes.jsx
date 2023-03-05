// https://codesandbox.io/s/m7inl?file=/src/App.js:452-781
// https://www.youtube.com/watch?v=2kTQZVzkXgI
// https://stackoverflow.com/questions/67555786/custom-buffergeometry-in-react-three-fiber
// https://github.com/bicarbon8/QuadSphere/blob/in-javascript/src/components/shapes.tsx
import { useThree } from '@react-three/fiber'
import { useDrag } from '@use-gesture/react'

const context = createContext(null)
const Circle = forwardRef(
  ({ children, opacity = 1, radius = 0.05, segments = 16, color = '#ff1050', ...props }, ref) => (
    <mesh ref={ref} {...props}>
      <circleGeometry args={[radius, segments]} />
      <meshBasicMaterial transparent={opacity < 1} opacity={opacity} color={color} />
      {children}
    </mesh>
  ),
)

function Nodes(positions) {
  const [nodes, set] = useState([])
  const group = useRef()
  return (
    <context.Provider value={set}>
      <group ref={group}>
        {positions.positions.map((position, index) => (
          <Node key={index} position={[position.x, position.y, 0]} />
        ))}
      </group>
    </context.Provider>
  )
}

const Node = forwardRef(({ color = 'black', name, position = [0, 0, 0], ...props }, ref) => {
  // const set = useContext(context);
  const { size, camera } = useThree()
  const [pos, setPos] = useState(() => new THREE.Vector3(...position))
  const state = useMemo(() => ({ position: pos }), [pos])
  const [hovered, setHovered] = useState(false)
  useEffect(() => void (document.body.style.cursor = hovered ? 'grab' : 'auto'), [hovered])
  const bind = useDrag(({ down, xy: [x, y] }) => {
    document.body.style.cursor = down ? 'grabbing' : 'grab'
    setPos(
      new THREE.Vector3((x / size.width) * 2 - 1, -(y / size.height) * 2 + 1, 0)
        .unproject(camera)
        .multiply({ x: 1, y: 1, z: 0 })
        .clone(),
    )
  })
  return (
    <Circle
      ref={ref}
      {...bind()}
      opacity={0.2}
      radius={0.5}
      color={color}
      position={pos}
      {...props}
    >
      <Circle
        radius={0.1}
        position={[0, 0, 0.1]}
        onPointerOver={() => setHovered(true)}
        onPointerOut={() => setHovered(false)}
        color={hovered ? '#ff1050' : color}
      ></Circle>
    </Circle>
  )
})