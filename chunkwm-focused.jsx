export const command = "/Users/shsiang/Documents/CS/desktop/focused"

export const refreshFrequency = 300

export const render = ({ output }) => {
  return (
    <div className="screen">
      <div className="chunkwm-focused">{ output }</div>
    </div>
  )
}
